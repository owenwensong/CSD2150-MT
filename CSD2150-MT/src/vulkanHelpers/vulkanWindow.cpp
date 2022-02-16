/*!*****************************************************************************
 * @file    vulkanWindow.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 FEB 2022
 * @brief   This is the implementation of the vulkan window class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/vulkanWindow.h>
#include <vulkan/vulkan_win32.h>
#include <iostream> // for wcout
#include <vector>
#include <span>

// *****************************************************************************
// *********************************************** NON CLASS STATIC HELPERS ****

VkSurfaceFormatKHR selectSurfaceFormat(VkPhysicalDevice VKDevice, 
                                       VkSurfaceKHR VKSurface, 
                                       std::span<const VkFormat> RequestFormats, 
                                       VkColorSpaceKHR RequestColorSpace) noexcept
{
    uint32_t Count{ 0 };
    if (VkResult tmpRes{ vkGetPhysicalDeviceSurfaceFormatsKHR(VKDevice, VKSurface, &Count, nullptr) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to get Physical Device Surface Formats Count"sv, true);
        return {};// undefined
    }
    std::vector<VkSurfaceFormatKHR> SurfaceFormats(Count);
    if (VkResult tmpRes{ vkGetPhysicalDeviceSurfaceFormatsKHR(VKDevice, VKSurface, &Count, SurfaceFormats.data()) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to get Physical Device Surface Formats"sv, true);
        return {};// undefined
    }

    if (SurfaceFormats.size() == 1)
    {
        if (SurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
        {   // any format is available ???? that's a weird way of saying ALL
            return VkSurfaceFormatKHR{ .format{ RequestFormats[0] }, .colorSpace{ RequestColorSpace } };
        }
    }
    else
    {
        for (VkFormat const& R : RequestFormats)
        {
            for (VkSurfaceFormatKHR const& S : SurfaceFormats)
            {
                if (R == S.format && S.colorSpace == RequestColorSpace)return S;
            }
        }
    }

    // No point in searching another format (is it guaranteed to be at least 1?)
    return SurfaceFormats[0];
}

VkFormat SelectDepthFormat(VkPhysicalDevice VKDevice, 
                           std::span<const VkFormat> RequestFormats, 
                           VkImageTiling Tiling, 
                           VkFormatFeatureFlags Features) //noexcept ??????
{
    for (VkFormat Format : RequestFormats)
    {
        VkFormatProperties Props{};
        vkGetPhysicalDeviceFormatProperties(VKDevice, Format, &Props);
        if (VK_IMAGE_TILING_LINEAR == Tiling && (Props.linearTilingFeatures & Features) == Features)
        {
            return Format;
        }
        else if (VK_IMAGE_TILING_OPTIMAL == Tiling && (Props.optimalTilingFeatures & Features) == Features)
        {
            return Format;
        }
    }
    // Unable to locate a good Z buffer format... ??????????
    assert(false);
    return RequestFormats[0];
}

VkPresentModeKHR SelectPresentMode(VkPhysicalDevice VKDevice, 
                                   VkSurfaceKHR VKSurface, 
                                   std::span<const VkPresentModeKHR> request_modes)
{
    uint32_t Count{ 0 };
    if (VkResult tmpRes{ vkGetPhysicalDeviceSurfacePresentModesKHR(VKDevice, VKSurface, &Count, nullptr) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to get Physical Device Surface Present Modes Count, Falling back to VK_PRESENT_MODE_FIFO_KHR"sv);
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    std::vector<VkPresentModeKHR> PresentModes(Count);
    if (VkResult tmpRes{ vkGetPhysicalDeviceSurfacePresentModesKHR(VKDevice, VKSurface, &Count, PresentModes.data()) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to get Physical Device Surface Present Modes, Falling back to VK_PRESENT_MODE_FIFO_KHR"sv);
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    for (VkPresentModeKHR const& R : request_modes)
    {
        for (VkPresentModeKHR const& P : PresentModes)
        {
            if (R == P)return P;
        }
    }
    printWarning("Requested present mode unavailable, falling back to VK_PRESENT_MODE_FIFO_KHR"sv);
    return VK_PRESENT_MODE_FIFO_KHR;
}

// *****************************************************************************

vulkanWindow::vulkanWindow(std::shared_ptr<vulkanDevice>& Device,
                           windowSetup const& Setup)
{   // not using initializer list because idk if it will be compatible in the future
    Initialize(Device, Setup);
    std::wcout << Setup.m_Title
               << L" | Window creation status: "sv
               << (m_bfInitializeOK ? L"OK"sv : L"BAD"sv)
               << std::endl;
}

vulkanWindow::~vulkanWindow()
{
    if (!m_Device)return;
    if (auto& VKI{ m_Device->getVKInst() }; VKI)
    {
        vkDestroySurfaceKHR(VKI->m_VkHandle, m_VKSurface, VKI->m_pVKAllocator);
    }
}

bool vulkanWindow::OK() const noexcept
{
    return m_bfInitializeOK ? true : false;
}

bool vulkanWindow::Initialize(std::shared_ptr<vulkanDevice>& Device, 
                              windowSetup const& Setup)
{
    m_Device = Device;
    m_bfClearOnRender = Setup.m_bClearOnRender ? 1 : 0;
    m_VKClearValue[0] = VkClearValue
    { .color{ .float32{  
        Setup.m_ClearColorR,
        Setup.m_ClearColorG,
        Setup.m_ClearColorB,
        Setup.m_ClearColorA
    } } };

    std::shared_ptr<vulkanInstance>& Instance{ m_Device->getVKInst() };

    // Set up system window
    if (false == m_windowsWindow.createWindow(Setup))
    {
        printWarning("FAILED TO CREATE WINDOWS WINDOW"sv, true);
        return false;
    }
    // same as OK, but I need the HWND to set data
    if (HWND winHandle{ m_windowsWindow.getSystemWindowHandle() }; winHandle != nullptr)
    {
        SetWindowLongPtr(winHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }
    else
    {
        printWarning("HWND WAS SOMEHOW INVALID DESPITE OK WINDOW"sv, true);
        return false;
    }

    // Create the surface
    {
        auto pFNVKCreateWin32Surface
        {
            reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>
            (
                vkGetInstanceProcAddr(Instance->m_VkHandle, "vkCreateWin32SurfaceKHR")
            )
        };
        if (nullptr == pFNVKCreateWin32Surface)
        {
            printWarning("Vulkan Driver missing the VK_KHR_win32_surface extension"sv, true);
            return false;
        }

        VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo
        {
            .sType      { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
            .pNext      { nullptr },
            .flags      { 0 },
            .hinstance  { GetModuleHandle(NULL) },
            .hwnd       { m_windowsWindow.getSystemWindowHandle() }
        };
        if (VkResult tmpRes{ pFNVKCreateWin32Surface(Instance->m_VkHandle, &SurfaceCreateInfo, Instance->m_pVKAllocator, &m_VKSurface)}; tmpRes != VK_SUCCESS)
        {
            printVKWarning(tmpRes, "Vulkan Failed to create the window surface"sv, true);
            return false;
        }
    }

    // Check to see if the selected queue supports presentation? Done here 
    // instead of in filtering phase of vulkanDevice collectPhysicalDevices
    // because the function needs the queueFamilyIndex and Surface to be
    // created already, so just ostrich it. Most devices should support it!
    {
        VkBool32 res{ VK_FALSE };
        if (VkResult tmpRes{ vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->m_VKPhysicalDevice, m_Device->m_MainQueueIndex, m_VKSurface, &res) }; tmpRes != VK_SUCCESS)
        {
            printVKWarning(tmpRes, "Error retrieving Physical Device Surface Support"sv, true);
            res = VK_FALSE; // make doubly sure it will exit later
        }
        if (res != VK_TRUE)
        {
            printWarning("Error no WSI support on physical device"sv, true);
            return false;
        }
    }

    // Select a surface format
    constexpr std::array requestSurfaceImageFormat
    {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM,
        VK_FORMAT_R8G8B8_UNORM
    };

    constexpr VkColorSpaceKHR requestSurfaceColorSpace{ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    m_VKSurfaceFormat = selectSurfaceFormat
    (
        m_Device->m_VKPhysicalDevice,
        m_VKSurface,
        requestSurfaceImageFormat,
        requestSurfaceColorSpace
    );

    if (m_VKSurfaceFormat.format == VkFormat::VK_FORMAT_UNDEFINED)
    {
        printWarning("SURFACE FORMAT SELECTION RETURNED UNDEFINED!"sv, true);
        return false;
    }

    // Select Depth Format
    constexpr std::array requestDepthFormats
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    m_VKDepthFormat = SelectDepthFormat
    (
        m_Device->m_VKPhysicalDevice,
        requestDepthFormats,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    // Select Present Mode
    constexpr std::array PresentModes
    {   // ????????????????????????
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_FIFO_KHR
    };

    m_VKPresentMode = SelectPresentMode
    (
        m_Device->m_VKPhysicalDevice, 
        m_VKSurface, 
        PresentModes
    );

    // ?????????????????????????????????????????????????????????????????????
    // WHAT IS THE POINT OF THE PREVIOUS LINES THEN ????????????????????????
    // ?????????????????????????????????????????????????????????????????????
    if (false == Setup.m_bSyncOn)
    {
        m_VKPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else
    {
        if (m_VKPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            m_VKPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
    }



    // Create surface and stuff, returning now just to test
    m_bfInitializeOK = 1;
    return true;

}
