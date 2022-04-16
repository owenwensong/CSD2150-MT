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
#include <vulkanHelpers/vulkanPipeline.h>
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

struct vulkanPipelineData
{
  VkPipeline  m_Pipeline      { VK_NULL_HANDLE };
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

    // My own stuff, giving up on reading xGPU especially with only a few hours remaining

    void setFullscreen(bool fullscreenMode) noexcept;

    void toggleFullscreen() noexcept;

    /// @brief begin a frame, made similar to the way imgui does their calls,
    ///        must be called in order FrameBegin, FrameEnd, PageFlip
    ///        if FrameBegin returns false, don't end or pageFlip.
    /// @return command buffer if frame begin success, to send commands
    VkCommandBuffer FrameBegin();

    /// @brief submits queues
    void FrameEnd();

    /// @brief swaps front and back buffer
    void PageFlip();

    // PIPELINE INFO (CUSTOM)

    bool createPipelineInfo(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup);
    void destroyPipelineInfo(vulkanPipeline& inPipeline);

    // any created will be stored to be auto destroyed
    bool createAndSetPipeline(vulkanPipeline& pipelineCustomCreateInfo);

    void setUniform(vulkanPipeline& inPipeline, uint32_t shaderTarget, uint32_t uniformTarget, void* pData, size_t dataLen);

private:

    void updateDefaultViewportAndScissor() noexcept;

    bool CreateOrResizeWindow(/* Get width and height internally */) noexcept;
    bool CreateWindowSwapChain() noexcept;
    bool CreateDepthResources(VkExtent2D Extents) noexcept;
    bool CreateRenderPass(VkSurfaceFormatKHR& VKColorSurfaceFormat, VkFormat& VKDepthSurfaceFormat) noexcept;
    bool CreateWindowCommandBuffers() noexcept;
    bool CreateUniformDescriptorSetLayouts(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup) noexcept;
    bool CreateUniformBuffers(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup) noexcept;
    bool CreateUniformDescriptorSets(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup) noexcept;

    void DestroyRenderPass() noexcept;
    void DestroyPipelineData(vulkanPipelineData& inPipelineData) noexcept;
    void DestroyPipelines() noexcept;
    void DestroyUniformDescriptorSetLayouts(vulkanPipeline& outPipeline) noexcept;
    void DestroyUniformBuffers(vulkanPipeline& outPipeline) noexcept;
    void DestroyUniformDescriptorSets(vulkanPipeline& outPipeline) noexcept;

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
    //VkPipeline                          m_VKPipeline            {};
    std::unordered_map<vulkanPipeline*, vulkanPipelineData> m_VKPipelines{};
    VkSurfaceFormatKHR                  m_VKSurfaceFormat       {};
    VkFormat                            m_VKDepthFormat         {};
    VkPresentModeKHR                    m_VKPresentMode         {};
    uint32_t                            m_SemaphoreIndex        { 0 };
    uint32_t                            m_FrameIndex            { 0 };
    //int                                 m_BeginState            { 0 };
    //int                                 m_nCmds                 { 0 };
    VkViewport                          m_DefaultViewport       {};
    VkRect2D                            m_DefaultScissor        {};
    // mati_per_renderpass_map  // not going to do this until required...
    // mat_per_renderpass_map   // so much back and forth I cannot read this

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield                            m_bfClearOnRender : 1   { 0 };
    bitfield                            m_bfRebuildSwapChain : 1{ 0 };
    bitfield                            m_bfInitializeOK : 1    { 0 };
    bitfield                            m_bfFrameBeginState : 2 { 0 };// unused in release

};

#endif//VULKAN_WINDOW_HELPER_HEADER
