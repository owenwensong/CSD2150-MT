/*!*****************************************************************************
 * @file    vulkanWindow.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 FEB 2022
 * @brief   This is the interface of the vulkan window class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_WINDOW_HELPER_HEADER
#define VULKAN_WINDOW_HELPER_HEADER

#include <windowsHelpers/windowsWindow.h>
#include <vulkanHelpers/vulkanDevice.h>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <memory>
#include <array>

struct vulkanFrame
{
    VkCommandPool   m_VKCommandPool     {};
    VkCommandBuffer m_VKCommandBuffer   {};
    VkFence         m_VKFence           {};
    VkImage         m_VKBackBuffer      {};
    VkImageView     m_VKBackBufferView  {};
    VkFramebuffer   m_VKFramebuffer     {};
};

struct vulkanFrameSem
{
    VkSemaphore     m_VKImageAcquiredSemaphore  {};
    VkSemaphore     m_VKRenderCompleteSemaphore {};
};

class vulkanWindow
{
public:

    vulkanWindow() = default;
    vulkanWindow(std::shared_ptr<vulkanDevice>& Device,
                 windowSetup const& Setup);

    ~vulkanWindow();

    bool OK() const noexcept;

    bool Initialize(std::shared_ptr<vulkanDevice>& Device, 
                    windowSetup const& Setup);

private:

    bool CreateOrResizeWindow(/* Get width and height internally */) noexcept;
    bool CreateWindowSwapChain() noexcept;
    bool CreateDepthResources(VkExtent2D Extents) noexcept;

public: // all public, let whoever touch it /shrug

    windowsWindow                       m_windowsWindow         {};
    std::shared_ptr<vulkanDevice>       m_Device                {};
    VkSurfaceKHR                        m_VKSurface             {};
    std::array<VkClearValue, 2>         m_VKClearValue          { VkClearValue{.color{.float32{ 0.0f, 0.0f, 0.0f, 1.0f } } }, VkClearValue{ .depthStencil{ 1.0f, 0 } } };
    VkSwapchainKHR                      m_VKSwapchain           {};
    uint32_t                            m_ImageCount            { 2 };// default double buffer
    std::unique_ptr<vulkanFrame[]>      m_Frames                {};
    std::unique_ptr<vulkanFrameSem[]>   m_FrameSemaphores       {};
    VkImage                             m_VKDepthbuffer         {};
    VkImageView                         m_VKDepthbufferView     {};
    VkDeviceMemory                      m_VKDepthbufferMemory   {};
    VkRenderPass                        m_VKRenderPass          {};
    VkPipeline                          m_VKPipeline            {};
    VkSurfaceFormatKHR                  m_VKSurfaceFormat       {};
    VkFormat                            m_VKDepthFormat         {};
    VkPresentModeKHR                    m_VKPresentMode         {};
    uint32_t                            m_SemaphoreIndex        { 0 };
    uint32_t                            m_FrameIndex            { 0 };
    int                                 m_BeginState            { 0 };
    int                                 m_nCmds                 { 0 };
    VkViewport                          m_DefaultViewport       {};
    VkRect2D                            m_DefaultScissor        {};
    // mati_per_renderpass_map  // not going to do this until required...
    // mat_per_renderpass_map   // so much back and forth I cannot read this

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield                            m_bfClearOnRender : 1   { 0 };
    bitfield                            m_bfRebuildSwapChain : 1{ 0 };
    bitfield                            m_bfInitializeOK : 1    { 0 };

};

#endif//VULKAN_WINDOW_HELPER_HEADER
