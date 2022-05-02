/*!*****************************************************************************
 * @file    vulkanWindow.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 FEB 2022
 * @brief   This is the implementation of the vulkan window class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <handlers/windowHandler.h> // to destroy buffer
#include <vulkanHelpers/vulkanWindow.h>
#include <vulkan/vulkan_win32.h>
#include <iostream> // for wcout
#include <variant>  // descriptor variants
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

  Frame.m_VKFence = VK_NULL_HANDLE;
  Frame.m_VKCommandBuffer = VK_NULL_HANDLE;
  Frame.m_VKCommandPool = VK_NULL_HANDLE;

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
  if (!m_Device)return;// assume VKInst valid if this is valid
  VkAllocationCallbacks* pAllocator{ m_Device->m_pVKInst->m_pVKAllocator };

  vkDeviceWaitIdle(m_Device->m_VKDevice);

  if (m_Frames.get())
  {
    for (uint32_t i{ 0 }, t{ m_ImageCount }; i < t; ++i)
    {
      MinimalDestroyFrame(m_Device->m_VKDevice, m_Frames[i], pAllocator);
      MinimalDestroyFrameSemaphores(m_Device->m_VKDevice, m_FrameSemaphores[i], pAllocator);
    }
    m_Frames.reset();
    m_FrameSemaphores.reset();

    // Release the depth buffer (will exist if Framebuffer exists right?)
    vkDestroyImageView(m_Device->m_VKDevice, m_VKDepthbufferView, pAllocator);
    vkDestroyImage(m_Device->m_VKDevice, m_VKDepthbuffer, pAllocator);
    vkFreeMemory(m_Device->m_VKDevice, m_VKDepthbufferMemory, pAllocator);
    // leave the pointers invalid? Checks done on Framebuffer stuff anyway?
  }
  // Destroy render pass and pipeline
  DestroyRenderPass();
  DestroyPipelines();

  // Destroy the Swap Chain
  if (m_VKSwapchain)vkDestroySwapchainKHR(m_Device->m_VKDevice, m_VKSwapchain, pAllocator);

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
  { .color{.float32{
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
    if (VkResult tmpRes{ pFNVKCreateWin32Surface(Instance->m_VkHandle, &SurfaceCreateInfo, Instance->m_pVKAllocator, &m_VKSurface) }; tmpRes != VK_SUCCESS)
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
  { // ????????????????????????
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

  if (false == CreateOrResizeWindow())return false;

  m_bfInitializeOK = 1;
  return true;

}

void vulkanWindow::setFullscreen(bool fullscreenMode) noexcept
{
  m_windowsWindow.setFullscreen(fullscreenMode);
}

void vulkanWindow::toggleFullscreen() noexcept
{
  m_windowsWindow.setFullscreen(m_windowsWindow.m_bfFullscreen ? false : true);
}

void vulkanWindow::updateDefaultViewportAndScissor() noexcept
{
  m_DefaultScissor.offset.x = 0;
  m_DefaultScissor.offset.y = 0;
  m_DefaultScissor.extent.width = static_cast<uint32_t>(m_windowsWindow.getWidth());
  m_DefaultScissor.extent.height = static_cast<uint32_t>(m_windowsWindow.getHeight());
  m_DefaultViewport.width = static_cast<float>(m_DefaultScissor.extent.width);
  m_DefaultViewport.height = -static_cast<float>(m_DefaultScissor.extent.height);
  m_DefaultViewport.x = 0.0f;
  m_DefaultViewport.y = static_cast<float>(m_DefaultScissor.extent.height);
  m_DefaultViewport.minDepth = 0.0f;
  m_DefaultViewport.maxDepth = 1.0f;
}

bool vulkanWindow::CreateOrResizeWindow() noexcept
{
  return CreateWindowSwapChain() && CreateWindowCommandBuffers();
}

bool vulkanWindow::CreateWindowSwapChain() noexcept
{
  // Preserve old swapchain to create the new one
  VkSwapchainKHR const VKOldSwapChain{ m_VKSwapchain };
  m_VKSwapchain = VK_NULL_HANDLE;

  VkAllocationCallbacks* pAllocator{ m_Device->m_pVKInst->m_pVKAllocator };

  m_Device->waitForDeviceIdle();

  // Destroy old Framebuffer
  if (m_Frames.get())
  {
    for (uint32_t i{ 0 }, t{ m_ImageCount }; i < t; ++i)
    {
      MinimalDestroyFrame(m_Device->m_VKDevice, m_Frames[i], pAllocator);
      MinimalDestroyFrameSemaphores(m_Device->m_VKDevice, m_FrameSemaphores[i], pAllocator);
    }
    m_Frames.reset();         // Destroyed in the step before (changed from release)
    m_FrameSemaphores.reset();// Destroyed in the step before (changed from release)

    // Release the depth buffer (will exist if Framebuffer exists right?)
    vkDestroyImageView(m_Device->m_VKDevice, m_VKDepthbufferView, pAllocator);
    vkDestroyImage(m_Device->m_VKDevice, m_VKDepthbuffer, pAllocator);
    vkFreeMemory(m_Device->m_VKDevice, m_VKDepthbufferMemory, pAllocator);
    // leave the pointers invalid? Checks done on Framebuffer stuff anyway?
  }

  // Destroy render pass and pipeline
  DestroyRenderPass();
  //DestroyPipelines();// scissor and viewport are dynamic now

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
  if (false == CreateRenderPass(m_VKSurfaceFormat, m_VKDepthFormat))return false;

  // Create the Image Views
  {
    VkImageViewCreateInfo CreateInfo
    {
      .sType      { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
      .viewType   { VK_IMAGE_VIEW_TYPE_2D },
      .format     { m_VKSurfaceFormat.format },
      .components
      {
        .r{ VK_COMPONENT_SWIZZLE_R },
        .g{ VK_COMPONENT_SWIZZLE_G },
        .b{ VK_COMPONENT_SWIZZLE_B },
        .a{ VK_COMPONENT_SWIZZLE_A }
      },
      .subresourceRange
      {
        .aspectMask     { VK_IMAGE_ASPECT_COLOR_BIT },
        .baseMipLevel   { 0 },
        .levelCount     { 1 },
        .baseArrayLayer { 0 },
        .layerCount     { 1 }
      }
    };

    for (uint32_t i{ 0 }; i < m_ImageCount; ++i)
    {
      auto& Frame{ m_Frames[i] };
      CreateInfo.image = Frame.m_VKBackBuffer;

      if (VkResult tmpRes{ vkCreateImageView(m_Device->m_VKDevice, &CreateInfo, pAllocator, &Frame.m_VKBackBufferView) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create an Image View for a back buffer"sv, true);
        return false;
      }
    }

  }

  // Create Framebuffer
  {
    std::array<VkImageView, 2> Attachment;// ???
    VkFramebufferCreateInfo CreateInfo
    {
      .sType          { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO },
      .renderPass     { m_VKRenderPass },
      .attachmentCount{ static_cast<uint32_t>(Attachment.size()) },
      .pAttachments   { Attachment.data() },
      .width          { static_cast<uint32_t>(m_windowsWindow.getWidth()) },
      .height         { static_cast<uint32_t>(m_windowsWindow.getHeight()) },
      .layers         { 1 }
    };

    for (uint32_t i{ 0 }; i < m_ImageCount; ++i)
    {
      auto& Frame{ m_Frames[i] };
      Attachment[0] = Frame.m_VKBackBufferView;
      Attachment[1] = m_VKDepthbufferView;
      if (VkResult tmpRes{ vkCreateFramebuffer(m_Device->m_VKDevice, &CreateInfo, pAllocator, &Frame.m_VKFramebuffer) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create a Frame Buffer"sv, true);
        return false;
      }
    }
  }

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

bool vulkanWindow::CreateRenderPass(VkSurfaceFormatKHR& VKColorSurfaceFormat, VkFormat& VKDepthSurfaceFormat) noexcept
{
  std::array ColorAttachmentRef
  {
    VkAttachmentReference
    {
      .attachment { 0 },
      .layout     { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
    }
  };

  std::array DepthAttachmentRef
  {
    VkAttachmentReference
    {
      .attachment { 1 },
      .layout     { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
    }
  };

  std::array SubpassDesc
  {
    VkSubpassDescription
    {
      .pipelineBindPoint      { VK_PIPELINE_BIND_POINT_GRAPHICS },
      .colorAttachmentCount   { static_cast<uint32_t>(ColorAttachmentRef.size()) },
      .pColorAttachments      { ColorAttachmentRef.data() },
      .pDepthStencilAttachment{ DepthAttachmentRef.data() }
    }
  };

  std::array SubpassDependancy
  {
    VkSubpassDependency
    {
      .srcSubpass     { VK_SUBPASS_EXTERNAL },
      .dstSubpass     { 0 },   // VK_SUBPASS_CONTENTS_INLINE???
      .srcStageMask   { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT },
      .dstStageMask   { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT },
      .srcAccessMask  { 0 },
      .dstAccessMask  { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT }
    }
  };

  std::array AttachmentDesc
  {
    // Color Attachment
    VkAttachmentDescription
    {
      .format         { VKColorSurfaceFormat.format },
      .samples        { VK_SAMPLE_COUNT_1_BIT },
      .loadOp         { m_bfClearOnRender ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE },
      .storeOp        { VK_ATTACHMENT_STORE_OP_STORE },
      .stencilLoadOp  { VK_ATTACHMENT_LOAD_OP_DONT_CARE },
      .stencilStoreOp { VK_ATTACHMENT_STORE_OP_DONT_CARE },
      .initialLayout  { VK_IMAGE_LAYOUT_UNDEFINED },    // ???
      .finalLayout    { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }
    },
    // Depth Attachment
    VkAttachmentDescription
    {
      .format         { VKDepthSurfaceFormat },
      .samples        { VK_SAMPLE_COUNT_1_BIT },
      .loadOp         { VK_ATTACHMENT_LOAD_OP_CLEAR },
      .storeOp        { VK_ATTACHMENT_STORE_OP_DONT_CARE },
      .stencilLoadOp  { VK_ATTACHMENT_LOAD_OP_DONT_CARE },
      .stencilStoreOp { VK_ATTACHMENT_STORE_OP_DONT_CARE },
      .initialLayout  { VK_IMAGE_LAYOUT_UNDEFINED },    // ???
      .finalLayout    { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
    }
  };

  VkRenderPassCreateInfo CreateInfo
  {
    .sType          { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO },
    .attachmentCount{ static_cast<uint32_t>(AttachmentDesc.size()) },
    .pAttachments   { AttachmentDesc.data() },
    .subpassCount   { static_cast<uint32_t>(SubpassDesc.size()) },
    .pSubpasses     { SubpassDesc.data() },
    .dependencyCount{ static_cast<uint32_t>(SubpassDependancy.size()) },
    .pDependencies  { SubpassDependancy.data() }
  };

  if (VkResult tmpRes{ vkCreateRenderPass(m_Device->m_VKDevice, &CreateInfo, m_Device->m_pVKInst->m_pVKAllocator, &m_VKRenderPass) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "Unable to create a render pass for the window"sv, true);
    return false;
  }

  return true;

}

bool vulkanWindow::CreateWindowCommandBuffers() noexcept
{
  VkAllocationCallbacks* pAllocator{ m_Device->m_pVKInst->m_pVKAllocator };

  for (uint32_t i{ 0 }; i < m_ImageCount; ++i)
  {
    auto& Frame{ m_Frames[i] };

    {   // COMMAND POOLS
      VkCommandPoolCreateInfo CreateInfo
      {
        .sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
        .flags{ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT },
        .queueFamilyIndex{ m_Device->m_MainQueueIndex }
      };
      if (VkResult tmpRes{ vkCreateCommandPool(m_Device->m_VKDevice, &CreateInfo, pAllocator, &Frame.m_VKCommandPool) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create a Frame Command Pool"sv, true);
        return false;
      }
    }

    {   // COMMAND BUFFERS
      VkCommandBufferAllocateInfo CreateInfo
      {
        .sType      { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
        .commandPool{ Frame.m_VKCommandPool },
        .level      { VK_COMMAND_BUFFER_LEVEL_PRIMARY },
        .commandBufferCount{ 1 }
      };
      if (VkResult tmpRes{ vkAllocateCommandBuffers(m_Device->m_VKDevice, &CreateInfo, &Frame.m_VKCommandBuffer) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create a Frame Command Buffer"sv, true);
        return false;
      }
    }

    {   // FENCES
      VkFenceCreateInfo CreateInfo
      {
        .sType{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO },
        .flags{ VK_FENCE_CREATE_SIGNALED_BIT }
      };
      if (VkResult tmpRes{ vkCreateFence(m_Device->m_VKDevice, &CreateInfo, pAllocator, &Frame.m_VKFence) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create a Frame Fence"sv, true);
        return false;
      }
    }

    {   // SEMAPHORES
      auto& FrameSemaphores{ m_FrameSemaphores[i] };
      VkSemaphoreCreateInfo CreateInfo
      {
        .sType{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO }
      };
      if (VkResult tmpRes{ vkCreateSemaphore(m_Device->m_VKDevice, &CreateInfo, pAllocator, &FrameSemaphores.m_VKImageAcquiredSemaphore) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create a Frame Image Semaphore"sv, true);
        return false;
      }
      if (VkResult tmpRes{ vkCreateSemaphore(m_Device->m_VKDevice, &CreateInfo, pAllocator, &FrameSemaphores.m_VKRenderCompleteSemaphore) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create a Frame Render Semaphore"sv, true);
        return false;
      }
    }

  }

  return true;

}

bool vulkanWindow::CreateUniformDescriptorSetLayouts(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup) noexcept
{
  std::vector<VkDescriptorSetLayoutBinding> uniformLayoutBindings;
  // Vertex shader uniforms
  uniformLayoutBindings.reserve(inSetup.m_UniformsVert.size());
  for (auto const& x : inSetup.m_UniformsVert)
  {
    uniformLayoutBindings.emplace_back
    (
      x.m_TypeBindingID,          // binding
      x.m_DescriptorType,         // descriptorType
      1,                          // descriptorCount
      VK_SHADER_STAGE_VERTEX_BIT, // stageFlags
      nullptr                     // pImmutableSamplers
    );
  }
  VkDescriptorSetLayoutCreateInfo layoutCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO },
    .bindingCount { static_cast<uint32_t>(uniformLayoutBindings.size()) },
    .pBindings    { uniformLayoutBindings.data() }
  };
  if (VkResult tmpRes{ vkCreateDescriptorSetLayout(m_Device->m_VKDevice, &layoutCreateInfo, m_Device->m_pVKInst->m_pVKAllocator, &outPipeline.m_DescriptorSetLayouts[0])}; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "could not create the vertex shader's descriptor set layout"sv, true);
    //return false;
  }

  // Fragment shader uniforms
  uniformLayoutBindings.clear();
  uniformLayoutBindings.reserve(inSetup.m_UniformsFrag.size());
  for (auto const& x : inSetup.m_UniformsFrag)
  {
    uniformLayoutBindings.emplace_back
    (
      x.m_TypeBindingID,            // binding
      x.m_DescriptorType,           // descriptorType
      1,                            // descriptorCount
      VK_SHADER_STAGE_FRAGMENT_BIT, // stageFlags
      nullptr                       // pImmutableSamplers
    );
  }
  layoutCreateInfo.bindingCount = static_cast<uint32_t>(uniformLayoutBindings.size());
  layoutCreateInfo.pBindings = uniformLayoutBindings.data();
  if (VkResult tmpRes{ vkCreateDescriptorSetLayout(m_Device->m_VKDevice, &layoutCreateInfo, m_Device->m_pVKInst->m_pVKAllocator, &outPipeline.m_DescriptorSetLayouts[1]) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "could not create the fragment shader's descriptor set layout"sv, true);
    //return false;
  }

  return true;
}

bool vulkanWindow::CreateUniformBuffers(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup) noexcept
{
  windowHandler* pWH{ windowHandler::getPInstance() };
  assert(pWH != nullptr);
  
  std::array<std::vector<vulkanPipeline::uniformInfo> const*, 2> refHelper
  {
    &inSetup.m_UniformsVert,
    &inSetup.m_UniformsFrag
  };

  for (size_t i{ 0 }, t{ refHelper.size() }; i < t; ++i)// for every shader
  {
    auto& DBufs{ outPipeline.m_DescriptorBuffers[i] };
    DBufs.resize(static_cast<size_t>(m_ImageCount) * outPipeline.m_DescriptorCounts[i]);
    for (size_t j{ 0 }, k{ refHelper[i]->size() }; j < k; ++j)// for every uniform
    {
      if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == refHelper[i][0][j].m_DescriptorType)
      {
        continue;
      }
      vulkanBuffer::Setup BufferSetup
      {
        .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Uniform },
        .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Uniform },
        .m_Count      { 1 },
        .m_ElemSize   { refHelper[i][0][j].m_TypeSize }
      };
      for (size_t l{ 0 }, m{ m_ImageCount }; l < m; ++l)// for every frame
      {
        size_t idx{ outPipeline.m_DescriptorCounts[i] * l + j};
        if (false == pWH->createBuffer(DBufs[idx], BufferSetup))
        {
          printWarning("Failed to create a uniform buffer for a shader"sv);
        }
      }
    }
  }

  return true;
}

bool vulkanWindow::CreateUniformDescriptorSets(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup) noexcept
{

  std::array<std::vector<vulkanPipeline::uniformInfo> const*, 2> refHelper
  {
    &inSetup.m_UniformsVert,
    &inSetup.m_UniformsFrag
  };

  std::array<std::vector<vulkanTexture*> const*, 2> refTexHelper
  {
    &inSetup.m_pTexturesVert,
    &inSetup.m_pTexturesFrag
  };

  outPipeline.m_DescriptorSets.resize(m_ImageCount);
  std::scoped_lock lock{ m_Device->m_LockedVKDescriptorPool };
  VkDescriptorSetAllocateInfo allocInfo
  {
    .sType{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO },
    .descriptorPool     { m_Device->m_LockedVKDescriptorPool.get() },
    .descriptorSetCount { static_cast<uint32_t>(outPipeline.m_DescriptorSetLayouts.size()) },
    .pSetLayouts        { outPipeline.m_DescriptorSetLayouts.data()}
  };
  for (size_t l{ 0 }, m{ m_ImageCount }; l < m; ++l)
  {
    auto& DSets{ outPipeline.m_DescriptorSets[l] };
    if (VkResult tmpRes{ vkAllocateDescriptorSets(m_Device->m_VKDevice, &allocInfo, DSets.data()) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, "failed to create a uniform descriptor set"sv, true);
      return false;
    }

    for (size_t i{ 0 }, t{ refHelper.size() }; i < t; ++i)// for every shader
    {
      VkDescriptorSet& DSet{ DSets[i] };
      std::vector<vulkanTexture*> const& pTexures{ *refTexHelper[i] };

      std::vector<std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>> bufferInfos;
      std::vector<VkWriteDescriptorSet> descriptorWrites;
      // ensure pointer validity by making sure no reallocs take place
      bufferInfos.reserve(outPipeline.m_DescriptorCounts[i]);      
      descriptorWrites.reserve(outPipeline.m_DescriptorCounts[i]);

      size_t samplerID{ 0 };

      for (size_t j{ 0 }, k{ refHelper[i]->size() }; j < k; ++j)// for every uniform
      {
        size_t bufferIndex{ outPipeline.m_DescriptorCounts[i] * l + j };

        bool isSampler{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == refHelper[i][0][j].m_DescriptorType };

        if (false == isSampler)
        {
          bufferInfos.emplace_back(VkDescriptorBufferInfo
          {
            .buffer { outPipeline.m_DescriptorBuffers[i][bufferIndex].m_Buffer },
            .offset { 0 },
            .range  { refHelper[i][0][j].m_TypeSize }
          });
        }
        else if (vulkanTexture* pTex{ pTexures[samplerID++] }; pTex != nullptr)
        {
          bufferInfos.emplace_back(VkDescriptorImageInfo{
            .sampler    { pTex->m_Sampler },
            .imageView  { pTex->m_View },
            .imageLayout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
          });
        }
        else
        {
          printWarning("a provided texture was nullptr"sv, true);
          return false;
        }
        
        descriptorWrites.emplace_back(VkWriteDescriptorSet
        {
          .sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
          .dstSet           { DSet },
          .dstBinding       { refHelper[i][0][j].m_TypeBindingID },
          .dstArrayElement  { 0 },
          .descriptorCount  { 1 },
          .descriptorType   { refHelper[i][0][j].m_DescriptorType },
          .pImageInfo       { isSampler ? &std::get<1>(bufferInfos.back()) : VK_NULL_HANDLE },
          .pBufferInfo      { isSampler ? VK_NULL_HANDLE : &std::get<0>(bufferInfos.back()) },
          .pTexelBufferView { VK_NULL_HANDLE }
        });
      }

      vkUpdateDescriptorSets(m_Device->m_VKDevice, outPipeline.m_DescriptorCounts[i], descriptorWrites.data(), 0, nullptr);

    }

  }
    
  return true;
}

void vulkanWindow::DestroyRenderPass() noexcept
{
  if (!m_Device)return;// assume OK instance if has device
  if (m_VKRenderPass)vkDestroyRenderPass(m_Device->m_VKDevice, m_VKRenderPass, m_Device->m_pVKInst->m_pVKAllocator);
  m_VKRenderPass = VK_NULL_HANDLE;
}

void vulkanWindow::DestroyPipelineData(vulkanPipelineData& inPipelineData) noexcept
{
  if (inPipelineData.m_Pipeline != VK_NULL_HANDLE)
  {
    vkDestroyPipeline(m_Device->m_VKDevice, inPipelineData.m_Pipeline, m_Device->m_pVKInst->m_pVKAllocator);
    inPipelineData.m_Pipeline = VK_NULL_HANDLE;
  }
}

void vulkanWindow::DestroyPipelines() noexcept
{
  for (decltype(m_VKPipelines)::iterator i{ m_VKPipelines.begin() }, t{ m_VKPipelines.end() }; i != t; ++i)
  {
    DestroyPipelineData(i->second);
  }
  m_VKPipelines.clear();
}

void vulkanWindow::DestroyUniformDescriptorSetLayouts(vulkanPipeline& outPipeline) noexcept
{
  for (auto& x : outPipeline.m_DescriptorSetLayouts)
  {
    if (x == VK_NULL_HANDLE)continue;
    vkDestroyDescriptorSetLayout(m_Device->m_VKDevice, x, m_Device->m_pVKInst->m_pVKAllocator);
    x = VK_NULL_HANDLE;
  }
}

void vulkanWindow::DestroyUniformBuffers(vulkanPipeline& outPipeline) noexcept
{
  windowHandler* pWH{ windowHandler::getPInstance() };
  assert(pWH != nullptr);
  for (auto& shaderBuffers : outPipeline.m_DescriptorBuffers)
  {
    for (auto& shaderBuf : shaderBuffers)pWH->destroyBuffer(shaderBuf);
    shaderBuffers.clear();
  }
}

void vulkanWindow::DestroyUniformDescriptorSets(vulkanPipeline& outPipeline) noexcept
{
  std::scoped_lock lock{ m_Device->m_LockedVKDescriptorPool };
  for (auto& DSets : outPipeline.m_DescriptorSets)
  {
    if (VkResult tmpRes{ vkFreeDescriptorSets(m_Device->m_VKDevice, m_Device->m_LockedVKDescriptorPool.get(), static_cast<uint32_t>(DSets.size()), DSets.data())}; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, "failed to create a uniform descriptor set"sv, true);
    }
  }
  outPipeline.m_DescriptorSets.clear();
}

VkCommandBuffer vulkanWindow::FrameBegin()
{
  if (m_windowsWindow.isMinimized())return VK_NULL_HANDLE;
  // will fail if was not 0 before starting
  assert(!m_bfFrameBeginState && (m_bfFrameBeginState += 2));

  // resize the window if needed
  if (m_windowsWindow.isResized())
  {
    m_Device->waitForDeviceIdle();
    CreateOrResizeWindow();// error strings alr printed here
    m_windowsWindow.resetResized();
  }

  // sync up with previous frame
  {
    auto& Frame{ m_Frames[m_FrameIndex] };
    auto& FrameSem{ m_FrameSemaphores[m_SemaphoreIndex] };

    // busy way for previous frame to finish rendering
    for (;;)
    {
      VkResult tmpRes{ vkWaitForFences(m_Device->m_VKDevice, 1, &Frame.m_VKFence, VK_TRUE, 100) };
      switch (tmpRes)
      {
      case VK_SUCCESS: break;
      case VK_TIMEOUT: continue;
      default:
        printVKWarning(tmpRes, "Failed to wait?"sv, true);
        assert(false);
        break;
      }
      break;
    }

    if (VkResult tmpRes{ vkAcquireNextImageKHR(m_Device->m_VKDevice, m_VKSwapchain, UINT64_MAX, FrameSem.m_VKImageAcquiredSemaphore, VK_NULL_HANDLE, &m_FrameIndex) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, "vkAcquireNextImageKHR failed?"sv, true);
      assert(false);
    }

  }

  auto& Frame{ m_Frames[m_FrameIndex] };

  // Reset the command buffer
  {
    if (VkResult tmpRes{ vkResetCommandPool(m_Device->m_VKDevice, Frame.m_VKCommandPool, 0) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, "vkResetCommandPool failed?"sv, true);
      assert(false);
    }

    VkCommandBufferBeginInfo CommandBufferBeginInfo
    {
      .sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
      .flags{ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT }
    };

    if (VkResult tmpRes{ vkBeginCommandBuffer(Frame.m_VKCommandBuffer, &CommandBufferBeginInfo) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, "vkBeginCommandBuffer failed?"sv, true);
      assert(false);
    }
  }

  // setup the renderpass
  VkRenderPassBeginInfo RenderPassBeginInfo
  {
    .sType          { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO },
    .renderPass     { m_VKRenderPass },
    .framebuffer    { Frame.m_VKFramebuffer },
    .renderArea
    {
      .extent
      {
        .width  { static_cast<uint32_t>(m_windowsWindow.getWidth()) },
        .height { static_cast<uint32_t>(m_windowsWindow.getHeight()) }
      }
    },
    .clearValueCount{ m_bfClearOnRender ? static_cast<uint32_t>(m_VKClearValue.size()) : 0u},
    .pClearValues   { m_bfClearOnRender ? m_VKClearValue.data() : nullptr}
  };
  vkCmdBeginRenderPass(Frame.m_VKCommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  // set the default viewport
  updateDefaultViewportAndScissor();

  vkCmdSetScissor(Frame.m_VKCommandBuffer, 0, 1, &m_DefaultScissor);
  vkCmdSetViewport(Frame.m_VKCommandBuffer, 0, 1, &m_DefaultViewport);


  return Frame.m_VKCommandBuffer;
}

void vulkanWindow::FrameEnd()
{
  // will fail if was not 2 before starting
  assert(--m_bfFrameBeginState);

  auto& Frame{ m_Frames[m_FrameIndex] };
  auto& FrameSem{ m_FrameSemaphores[m_SemaphoreIndex] };

  // officially end the pass
  vkCmdEndRenderPass(Frame.m_VKCommandBuffer);

  // officially end the commands
  if (VkResult tmpRes{ vkEndCommandBuffer(Frame.m_VKCommandBuffer) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "vkEndCommandBuffer failed?"sv, true);
    assert(false);
  }

  // Reset the frame fence to know when we are finished with the frame
  if (VkResult tmpRes{ vkResetFences(m_Device->m_VKDevice, 1, &Frame.m_VKFence) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "vkResetFences failed?"sv, true);
    assert(false);
  }

  // Submit the frame to the queue for processing 
  VkPipelineStageFlags WaitStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo SubmitInfo
  {
    .sType                  { VK_STRUCTURE_TYPE_SUBMIT_INFO },
    .waitSemaphoreCount     { 1 },
    .pWaitSemaphores        { &FrameSem.m_VKImageAcquiredSemaphore },
    .pWaitDstStageMask      { &WaitStage },
    .commandBufferCount     { 1 },
    .pCommandBuffers        { &Frame.m_VKCommandBuffer },
    .signalSemaphoreCount   { 1 },
    .pSignalSemaphores      { &FrameSem.m_VKRenderCompleteSemaphore }
  };

  // supposedly as good, if not better than lock_guard
  std::scoped_lock Lk{ m_Device->m_VKMainQueue };
  if (VkResult tmpRes{ vkQueueSubmit(m_Device->m_VKMainQueue.get(), 1, &SubmitInfo, Frame.m_VKFence) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "vkQueueSubmit failed?"sv, true);
    assert(false);
  }
}

void vulkanWindow::PageFlip()
{
  // will fail if was not 1 before starting
  assert(!(--m_bfFrameBeginState));

  auto& Frame{ m_Frames[m_FrameIndex] };
  auto& FrameSem{ m_FrameSemaphores[m_SemaphoreIndex] };

  uint32_t PresetIndex{ m_FrameIndex };
  VkPresentInfoKHR Info
  {
    .sType              { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR },
    .waitSemaphoreCount { 1 },
    .pWaitSemaphores    { &FrameSem.m_VKRenderCompleteSemaphore },
    .swapchainCount     { 1 },
    .pSwapchains        { &m_VKSwapchain },
    .pImageIndices      { &PresetIndex }
  };

  std::scoped_lock Lk{ m_Device->m_VKMainQueue };
  if (VkResult tmpRes{ vkQueuePresentKHR(m_Device->m_VKMainQueue.get(), &Info) }; tmpRes != VK_SUCCESS)
  {
    switch (tmpRes)
    {
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
      if (CreateOrResizeWindow())break;
    default:
      printVKWarning(tmpRes, "vkQueuePresentKHR Failed?"sv, true);
      assert(false);
      break;
    }
  }

  m_FrameIndex = (++m_FrameIndex) % m_ImageCount;
  m_SemaphoreIndex = (++m_SemaphoreIndex) % m_ImageCount;

}

bool vulkanWindow::createPipelineInfo(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup)
{
  windowHandler* pWH{ windowHandler::getPInstance() };
  assert(pWH != nullptr);

  destroyPipelineInfo(outPipeline); // make sure it's fresh boi

  if (inSetup.m_VertexBindingMode == vulkanPipeline::E_VERTEX_BINDING_MODE::UNDEFINED)
  {
    printWarning("Cannot create the pipeline layout. Vertex binding mode not defined."sv, true);
    return false;
  }

  outPipeline.m_ShaderVert = pWH->createShaderModule(inSetup.m_PathShaderVert.data());
  if (outPipeline.m_ShaderVert == VK_NULL_HANDLE)
  {
    std::string msg{ "Cannot create pipeline layout. Vertex shader failed to create from: " };
    msg.append(inSetup.m_PathShaderVert);
    printWarning(msg, true);
    destroyPipelineInfo(outPipeline);
    return false;
  }
  outPipeline.m_ShaderFrag = pWH->createShaderModule(inSetup.m_PathShaderFrag.data());
  if (outPipeline.m_ShaderFrag == VK_NULL_HANDLE)
  {
    std::string msg{ "Cannot create pipeline layout. Fragment shader failed to create from: " };
    msg.append(inSetup.m_PathShaderFrag);
    printWarning(msg, true);
    destroyPipelineInfo(outPipeline);
    return false;
  }

  outPipeline.m_DescriptorCounts[0] = static_cast<uint32_t>(inSetup.m_UniformsVert.size());
  outPipeline.m_DescriptorCounts[1] = static_cast<uint32_t>(inSetup.m_UniformsFrag.size());
  
  {
    size_t numSamplers{ 0 };
    for (auto const& x : inSetup.m_UniformsVert)
    {
      if (x.m_DescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      {
        ++numSamplers;
      }
    }
    if (numSamplers != inSetup.m_pTexturesVert.size())
    {
      printWarning("uniform info number of samplers and provided textures mismatched (vert)!"sv, true);
      return false;
    }
  }

  {
    size_t numSamplers{ 0 };
    for (auto const& x : inSetup.m_UniformsFrag)
    {
      if (x.m_DescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      {
        ++numSamplers;
      }
    }
    if (numSamplers != inSetup.m_pTexturesFrag.size())
    {
      printWarning("uniform info number of samplers and provided textures mismatched (frag)!"sv, true);
      return false;
    }
  }
  
  if (false == CreateUniformDescriptorSetLayouts(outPipeline, inSetup))
  {
    printWarning("Failed to create uniform descriptor set layouts"sv);
    return false;
  }
  if (false == CreateUniformBuffers(outPipeline, inSetup))
  {
    printWarning("Failed to create uniform buffers"sv);
    return false;
  }
  if (false == CreateUniformDescriptorSets(outPipeline, inSetup))
  {
    printWarning("Failed to create uniform descriptor sets"sv);
    return false;
  }

  { // local scope for VkPipelineLayout

    std::vector<VkPushConstantRange> PCRanges;
    PCRanges.reserve(2);
    uint32_t PCRangeOffset{ 0 };
    if (inSetup.m_PushConstantRangeVert.size != 0)
    {
      VkPushConstantRange& currPCRange{ PCRanges.emplace_back(inSetup.m_PushConstantRangeVert) };
      currPCRange.offset += PCRangeOffset;
      outPipeline.m_PushConstantOffsets[0] = currPCRange.offset;  // always 0 lol
      PCRangeOffset += currPCRange.size;
    }
    if (inSetup.m_PushConstantRangeFrag.size != 0)
    {
      VkPushConstantRange& currPCRange{ PCRanges.emplace_back(inSetup.m_PushConstantRangeFrag) };
      currPCRange.offset += PCRangeOffset;
      outPipeline.m_PushConstantOffsets[1] = currPCRange.offset;  // yikes
      PCRangeOffset += currPCRange.size;
    }

    VkPipelineLayoutCreateInfo CreateInfo
    {
      .sType{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO },
      .setLayoutCount         { static_cast<uint32_t>(outPipeline.m_DescriptorSetLayouts.size()) },
      .pSetLayouts            { outPipeline.m_DescriptorSetLayouts.data() },
      .pushConstantRangeCount { static_cast<uint32_t>(PCRanges.size()) },
      .pPushConstantRanges    { PCRanges.data() }
    };

    outPipeline.m_PipelineLayout = pWH->createPipelineLayout(CreateInfo);
    if (outPipeline.m_PipelineLayout == VK_NULL_HANDLE)
    {
      printWarning("Cannot create pipeline layout. PipelineLayout creation failed", true);
      destroyPipelineInfo(outPipeline);
      return false;
    }
  }

  outPipeline.m_ShaderStages[0] = VkPipelineShaderStageCreateInfo
  {
    .sType { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO },
    .stage { VK_SHADER_STAGE_VERTEX_BIT },
    .module{ outPipeline.m_ShaderVert },
    .pName { "main" }
  };
  outPipeline.m_ShaderStages[1] = VkPipelineShaderStageCreateInfo
  {
    .sType { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO },
    .stage { VK_SHADER_STAGE_FRAGMENT_BIT },
    .module{ outPipeline.m_ShaderFrag },
    .pName { "main" }
  };

  if (false == pWH->setupVertexInputInfo(outPipeline, inSetup))
  {
    destroyPipelineInfo(outPipeline);
    printWarning("could not create pipeline info, failed to setup vertex input info."sv, true);
    return false;
  }

  outPipeline.m_InputAssembly = VkPipelineInputAssemblyStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO },
    .topology								{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
    .primitiveRestartEnable	{ VK_FALSE }
  };

  outPipeline.m_Rasterizer = VkPipelineRasterizationStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO },
    .depthClampEnable				{ VK_FALSE },	// clamp instead of discarding stuff outside the near/far planes
    .rasterizerDiscardEnable{ VK_FALSE },
    .polygonMode						{ VK_POLYGON_MODE_FILL },
    .cullMode								{ VK_CULL_MODE_BACK_BIT },	// back face culling
    .frontFace							{ VK_FRONT_FACE_COUNTER_CLOCKWISE },
    .depthBiasEnable				{ VK_FALSE },
    .depthBiasConstantFactor{ 0.0f },
    .depthBiasClamp					{ 0.0f },
    .depthBiasSlopeFactor		{ 0.0f },
    .lineWidth							{ 1.0f }
  };

  outPipeline.m_Multisampling = VkPipelineMultisampleStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO },
    .rasterizationSamples		{ VK_SAMPLE_COUNT_1_BIT },
    .sampleShadingEnable		{ VK_FALSE },
    .minSampleShading				{ 1.0f },
    .pSampleMask						{ nullptr },
    .alphaToCoverageEnable	{ VK_FALSE },
    .alphaToOneEnable				{ VK_FALSE }
  };

  outPipeline.m_DepthStencilState = VkPipelineDepthStencilStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO },
    .pNext{ nullptr },
    .flags{ 0 },
    .depthTestEnable{ VK_TRUE },		// make this an option next time?
    .depthWriteEnable{ VK_TRUE },		// make this an option next time?
    .depthCompareOp{ VK_COMPARE_OP_LESS_OR_EQUAL },
    .depthBoundsTestEnable{ VK_TRUE },	// make this an option next time?
    .stencilTestEnable{ VK_TRUE },			// make this an option next time?
    .minDepthBounds{ 0.0f },
    .maxDepthBounds{ 1.0f }
  };

  outPipeline.m_ColorBlendAttachment = VkPipelineColorBlendAttachmentState
  {
    .blendEnable				{ VK_TRUE },		// should it be false?
    .srcColorBlendFactor{ VK_BLEND_FACTOR_SRC_ALPHA },
    .dstColorBlendFactor{ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA },
    .colorBlendOp				{ VK_BLEND_OP_ADD },
    .srcAlphaBlendFactor{ VK_BLEND_FACTOR_SRC_ALPHA },
    .dstAlphaBlendFactor{ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA },
    .alphaBlendOp{},
    .colorWriteMask
    {
      VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT
    }
  };

  outPipeline.m_ColorBlending = VkPipelineColorBlendStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO },
    .logicOpEnable	{ VK_TRUE },
    .logicOp				{ VK_LOGIC_OP_COPY },
    .attachmentCount{ 1 },
    .pAttachments		{ &outPipeline.m_ColorBlendAttachment },
    .blendConstants { 0.0f, 0.0f, 0.0f, 0.0f }	// ????
  };

  outPipeline.m_DynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
  outPipeline.m_DynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

  outPipeline.m_DynamicStateCreateInfo = VkPipelineDynamicStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO },
    .dynamicStateCount	{ static_cast<uint32_t>(outPipeline.m_DynamicStates.size()) },
    .pDynamicStates			{ outPipeline.m_DynamicStates.data() }
  };

  return true;
}

void vulkanWindow::destroyPipelineInfo(vulkanPipeline& inPipeline)
{
  windowHandler* pWH{ windowHandler::getPInstance() };
  assert(pWH != nullptr);
  DestroyUniformDescriptorSets(inPipeline);
  DestroyUniformBuffers(inPipeline);
  DestroyUniformDescriptorSetLayouts(inPipeline);
  pWH->destroyPipelineLayout(inPipeline.m_PipelineLayout);
  pWH->destroyShaderModule(inPipeline.m_ShaderFrag);
  pWH->destroyShaderModule(inPipeline.m_ShaderVert);
}

bool vulkanWindow::createAndSetPipeline(vulkanPipeline& pipelineCustomCreateInfo)
{
  if (pipelineCustomCreateInfo.m_PipelineLayout == VK_NULL_HANDLE)
  {
    printWarning("Cannot create pipeline with null pipelineLayout?"sv, true);
    return false;
  }

  updateDefaultViewportAndScissor();

  VkPipelineViewportStateCreateInfo viewportState
  {
      .sType{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO },
      .viewportCount{ 1 },
      .pViewports{ &m_DefaultViewport },
      .scissorCount{ 1 },
      .pScissors{ &m_DefaultScissor }
  };

  VkGraphicsPipelineCreateInfo CreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO },
    .stageCount         { static_cast<uint32_t>(pipelineCustomCreateInfo.m_ShaderStages.size()) },
    .pStages            { pipelineCustomCreateInfo.m_ShaderStages.data() },
    .pVertexInputState  { &pipelineCustomCreateInfo.m_VertexInputInfo },
    .pInputAssemblyState{ &pipelineCustomCreateInfo.m_InputAssembly },
    .pViewportState     { &viewportState },
    .pRasterizationState{ &pipelineCustomCreateInfo.m_Rasterizer },
    .pMultisampleState  { &pipelineCustomCreateInfo.m_Multisampling },
    .pDepthStencilState { &pipelineCustomCreateInfo.m_DepthStencilState },
    .pColorBlendState   { &pipelineCustomCreateInfo.m_ColorBlending },
    .pDynamicState      { &pipelineCustomCreateInfo.m_DynamicStateCreateInfo },
    .layout             { pipelineCustomCreateInfo.m_PipelineLayout },
    .renderPass         { m_VKRenderPass },
    .subpass            { 0 },
    .basePipelineHandle { VK_NULL_HANDLE },
    .basePipelineIndex  { -1 }
  };

  VkPipeline pipelineToSet{ VK_NULL_HANDLE };

  if (decltype(m_VKPipelines)::iterator found{ m_VKPipelines.find(&pipelineCustomCreateInfo) }; found != m_VKPipelines.end())
  {
    pipelineToSet = found->second.m_Pipeline;
  }
  else
  {
    if (VkResult tmpRes{ vkCreateGraphicsPipelines(m_Device->m_VKDevice, m_Device->m_VKPipelineCache, 1, &CreateInfo, m_Device->m_pVKInst->m_pVKAllocator, &pipelineToSet) }; tmpRes != VK_SUCCESS || pipelineToSet == VK_NULL_HANDLE)
    {
      printVKWarning(tmpRes, "Failed to create a pipeline!"sv, true);
      return false;
    }

    auto emplaceResult{ m_VKPipelines.emplace(&pipelineCustomCreateInfo, vulkanPipelineData{ .m_Pipeline{ pipelineToSet } }) };
    if (false == emplaceResult.second)
    {
      // I have no response, just pretend it never happened
      printWarning("failed to emplace pipeline"sv, true);
    }
    
  }

  auto& Frame{ m_Frames[m_FrameIndex] };

  // Bind pipeline
  vkCmdBindPipeline(Frame.m_VKCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineToSet);

  auto& frameDescriptorSets{ pipelineCustomCreateInfo.m_DescriptorSets[m_FrameIndex] };
  vkCmdBindDescriptorSets
  (
    Frame.m_VKCommandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineCustomCreateInfo.m_PipelineLayout,
    0,// first set
    static_cast<uint32_t>(frameDescriptorSets.size()),
    frameDescriptorSets.data(),
    0,
    nullptr
  );

  return true;

}

void vulkanWindow::setUniform(vulkanPipeline& inPipeline, uint32_t shaderTarget, uint32_t uniformTarget, void* pData, size_t dataLen)
{
  vulkanBuffer& targetBuffer{ inPipeline.m_DescriptorBuffers[shaderTarget][static_cast<size_t>(m_FrameIndex) * inPipeline.m_DescriptorCounts[shaderTarget] + uniformTarget]};
  assert(pData && dataLen <= targetBuffer.m_Settings.m_ElemSize * targetBuffer.m_Settings.m_Count);
  void* pDst{ nullptr };
  if (VkResult tmpRes{ vkMapMemory(m_Device->m_VKDevice, targetBuffer.m_BufferMemory, 0, dataLen, 0, &pDst)}; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "Failed to map memory"sv, true);
    return;
  }
  std::memcpy(pDst, pData, dataLen);
  vkUnmapMemory(m_Device->m_VKDevice, targetBuffer.m_BufferMemory);
}

void vulkanPipeline::pushConstant(VkCommandBuffer FCB, VkShaderStageFlags stageFlags, uint32_t offsetInto, uint32_t srcSize, const void* srcData)
{
  assert(FCB != VK_NULL_HANDLE);
  assert(srcSize && srcData);
  switch (stageFlags)
  {
  case VK_SHADER_STAGE_VERTEX_BIT:
    offsetInto += m_PushConstantOffsets[0];
    break;
  case VK_SHADER_STAGE_FRAGMENT_BIT:
    offsetInto += m_PushConstantOffsets[1];
    break;
  default: break;
  }
  vkCmdPushConstants(FCB, m_PipelineLayout, stageFlags, offsetInto, srcSize, srcData);
}
