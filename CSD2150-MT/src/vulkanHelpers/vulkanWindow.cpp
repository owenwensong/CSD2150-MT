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

void MinimalDestroyFrame(VkDevice VKDevice, vulkanFrame& Frame, VkAllocationCallbacks const* pAllocator) noexcept
{
    vkDestroyFence(VKDevice, Frame.m_VKFence, pAllocator);
    vkFreeCommandBuffers(VKDevice, Frame.m_VKCommandPool, 1, &Frame.m_VKCommandBuffer);
    vkDestroyCommandPool(VKDevice, Frame.m_VKCommandPool, pAllocator);

    Frame.m_VKFence         = VK_NULL_HANDLE;
    Frame.m_VKCommandBuffer = VK_NULL_HANDLE;
    Frame.m_VKCommandPool   = VK_NULL_HANDLE;

    vkDestroyImageView(VKDevice, Frame.m_VKBackBufferView, pAllocator);
    vkDestroyFramebuffer(VKDevice, Frame.m_VKFramebuffer, pAllocator);
}

void MinimalDestroyFrameSemaphores(VkDevice VKDevice, vulkanFrameSem& FrameSemaphores, VkAllocationCallbacks const* pAllocator) noexcept
{
    vkDestroySemaphore(VKDevice, FrameSemaphores.m_VKImageAcquiredSemaphore, pAllocator);
    vkDestroySemaphore(VKDevice, FrameSemaphores.m_VKRenderCompleteSemaphore, pAllocator);

    FrameSemaphores.m_VKImageAcquiredSemaphore = VK_NULL_HANDLE;
    FrameSemaphores.m_VKRenderCompleteSemaphore = VK_NULL_HANDLE;
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

bool vulkanWindow::CreateOrResizeWindow() noexcept
{
   return CreateWindowSwapChain();
}

bool vulkanWindow::CreateWindowSwapChain() noexcept
{
    // Preserve old swapchain to create the new one
    VkSwapchainKHR const VKOldSwapChain{ m_VKSwapchain };
    m_VKSwapchain = nullptr;

    VkAllocationCallbacks* pAllocator{ m_Device->m_pVKInst->m_pVKAllocator };

    if (VkResult tmpRes{ vkDeviceWaitIdle(m_Device->m_VKDevice)}; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to wait for device", true);
        // ???? Pretend there was no error ????
    }
    
    // Destroy old Framebuffer
    if (m_Frames.get())
    {
        for (uint32_t i{ 0 }, t{ m_ImageCount }; i < t; ++i)
        {
            MinimalDestroyFrame(m_Device->m_VKDevice, m_Frames[i], pAllocator);
            MinimalDestroyFrameSemaphores(m_Device->m_VKDevice, m_FrameSemaphores[i], pAllocator);
        }
        m_Frames.release();         // I guess release instead of reset because
        m_FrameSemaphores.release();// it is attached to VKOldSwapChain???

        // Release the depth buffer (will exist if Framebuffer exists right?)
        vkDestroyImageView(m_Device->m_VKDevice, m_VKDepthbufferView, pAllocator);
        vkDestroyImage(m_Device->m_VKDevice, m_VKDepthbuffer, pAllocator);
        vkFreeMemory(m_Device->m_VKDevice, m_VKDepthbufferMemory, pAllocator);
        // leave the pointers invalid? Checks done on Framebuffer stuff anyway?
    }

    // Destroy render pass and pipeline
    if (m_VKRenderPass)vkDestroyRenderPass(m_Device->m_VKDevice, m_VKRenderPass, pAllocator);
    m_VKRenderPass = VK_NULL_HANDLE;

    if (m_VKPipeline)vkDestroyPipeline(m_Device->m_VKDevice, m_VKPipeline, pAllocator);
    m_VKPipeline = VK_NULL_HANDLE;

    // Create the Swapchain
    {
        VkSwapchainCreateInfoKHR SwapChainInfo
        {
            .sType              { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR },
            .pNext              { nullptr },
            .flags              { 0 },
            .surface            { m_VKSurface },
            .minImageCount      { m_ImageCount },
            .imageFormat        { m_VKSurfaceFormat.format },
            .imageColorSpace    { m_VKSurfaceFormat.colorSpace },
            .imageExtent
            {
                .width  { static_cast<decltype(VkExtent2D::width)>(m_windowsWindow.getWidth()) },
                .height { static_cast<decltype(VkExtent2D::height)>(m_windowsWindow.getHeight()) }
            },
            .imageArrayLayers   { 1 },
            .imageUsage         { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT },
            .imageSharingMode   { VK_SHARING_MODE_EXCLUSIVE },
            .preTransform       { VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR },
            .compositeAlpha     { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR },
            .presentMode        { m_VKPresentMode },
            .clipped            { VK_TRUE },
            .oldSwapchain       { VKOldSwapChain }  // reused here :^D
        };

        VkSurfaceCapabilitiesKHR SurfaceCapabilities{};
        if (VkResult tmpRes{ vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->m_VKPhysicalDevice, m_VKSurface, &SurfaceCapabilities) }; tmpRes != VK_SUCCESS)
        {
            printVKWarning(tmpRes, "Failed to get the physical device surface capabilities"sv);
            // pretend no error?
        }
        else
        {
            if (m_ImageCount == 0 || m_ImageCount < SurfaceCapabilities.minImageCount)
            {
                m_ImageCount = SurfaceCapabilities.minImageCount;
            }
            else if (m_ImageCount > SurfaceCapabilities.maxImageCount)
            {
                m_ImageCount = SurfaceCapabilities.maxImageCount;
                printWarning("Reducing the number of usable buffers to render as device surface does not support as many as requested"sv);
            }

            if (SurfaceCapabilities.currentExtent.width != 0xFFFFFFFF)
            {
                SwapChainInfo.imageExtent.width = m_windowsWindow.m_Width = SurfaceCapabilities.currentExtent.width;
                SwapChainInfo.imageExtent.height = m_windowsWindow.m_Height = SurfaceCapabilities.currentExtent.height;
            }

        }

        
        if (false == CreateDepthResources(SwapChainInfo.imageExtent))return false;

        if (VkResult tmpRes{ vkCreateSwapchainKHR(m_Device->m_VKDevice, &SwapChainInfo, pAllocator, &m_VKSwapchain) }; tmpRes != VK_SUCCESS)
        {
            printVKWarning(tmpRes, "Failed to create the Swap Chain"sv, true);
            return false;
        }

        if (VkResult tmpRes{ vkGetSwapchainImagesKHR(m_Device->m_VKDevice, m_VKSwapchain, &m_ImageCount, NULL) }; tmpRes != VK_SUCCESS)
        {
            printVKWarning(tmpRes, "Failed to get the Swap Chain Image Count"sv, true);
            return false;
        }

        auto BackBuffers{ std::make_unique<VkImage[]>(m_ImageCount) };
        if (VkResult tmpRes{ vkGetSwapchainImagesKHR(m_Device->m_VKDevice, m_VKSwapchain, &m_ImageCount, BackBuffers.get()) }; tmpRes != VK_SUCCESS)
        {
            printVKWarning(tmpRes, "Failed to get the Swap Chain Images"sv, true);
            return false;
        }

        assert(m_Frames.get() == nullptr);
        m_Frames = std::make_unique<vulkanFrame[]>(m_ImageCount);
        m_FrameSemaphores = std::make_unique<vulkanFrameSem[]>(m_ImageCount);

        for (uint32_t i{ 0 }; i < m_ImageCount; ++i)
        {
            m_Frames[i].m_VKBackBuffer = BackBuffers[i];// ??? move? ???
        }
    }

    // Destroy the old Swap Chain
    if (VKOldSwapChain)vkDestroySwapchainKHR(m_Device->m_VKDevice, VKOldSwapChain, pAllocator);

    // Create the Render Pass
    // CONTINUE FROM 360 VK WINDOW CPP
    return true;
}

bool vulkanWindow::CreateDepthResources(VkExtent2D Extents) noexcept
{
    VkImageCreateInfo ImageInfo
    {
        .sType          { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO },
        .flags          { 0 },
        .imageType      { VK_IMAGE_TYPE_2D },
        .format         { m_VKDepthFormat },
        .extent
        {
            .width  { Extents.width },
            .height { Extents.height },
            .depth  { 1 }
        },
        .mipLevels      { 1 },
        .arrayLayers    { 1 },
        .samples        { VK_SAMPLE_COUNT_1_BIT },
        .tiling         { VK_IMAGE_TILING_OPTIMAL },
        .usage          { VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT },
        .sharingMode    { VK_SHARING_MODE_EXCLUSIVE },
        .initialLayout  { VK_IMAGE_LAYOUT_UNDEFINED } // ???
    };

    VkAllocationCallbacks* pAllocator{ m_Device->m_pVKInst->m_pVKAllocator };

    if (VkResult tmpRes{ vkCreateImage(m_Device->m_VKDevice, &ImageInfo, pAllocator, &m_VKDepthbuffer) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to create the depth buffer image"sv, true);
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_Device->m_VKDevice, m_VKDepthbuffer, &memRequirements);

    uint32_t MemoryIndex;
    if (false == m_Device->getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MemoryIndex))
    {
        printWarning("Failed to find the right type of memory to allocate the zbuffer"sv, true);
        return false;
    }
    VkMemoryAllocateInfo AllocInfo
    {
        .sType          { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO },
        .allocationSize { memRequirements.size },
        .memoryTypeIndex{ MemoryIndex }
    };

    if (VkResult tmpRes{ vkAllocateMemory(m_Device->m_VKDevice, &AllocInfo, pAllocator, &m_VKDepthbufferMemory) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to allocate memory for the zbuffer"sv, true);
        return false;
    }

    if (VkResult tmpRes{ vkBindImageMemory(m_Device->m_VKDevice, m_VKDepthbuffer, m_VKDepthbufferMemory, 0) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to bind the zbuffer with its image/memory"sv, true);
        return false;
    }

    VkImageViewCreateInfo ViewInfo
    {
        .sType      { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
        .image      { m_VKDepthbuffer },
        .viewType   { VK_IMAGE_VIEW_TYPE_2D },
        .format     { m_VKDepthFormat },
        .subresourceRange
        {
            .aspectMask     { VK_IMAGE_ASPECT_DEPTH_BIT },
            .baseMipLevel   { 0 },
            .levelCount     { 1 },
            .baseArrayLayer { 0 },
            .layerCount     { 1 }
        }
    };

    if (VkResult tmpRes{ vkCreateImageView(m_Device->m_VKDevice, &ViewInfo, pAllocator, &m_VKDepthbufferView) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to create the depth buffer view"sv, true);
        return false;
    }

    return true;

}
