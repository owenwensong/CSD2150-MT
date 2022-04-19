/*!*****************************************************************************
 * @file    graphicsHandler_vulkan.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    12 FEB 2022
 * @brief   This is the interface for the graphicsHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef GRAPHICS_HANDLER_VULKAN_HEADER
#define GRAPHICS_HANDLER_VULKAN_HEADER

#include <memory>   // std::shared_ptr
#include <filesystem>
#include <utility/Singleton.h>
#include <vulkanHelpers/vulkanInstance.h>
#include <vulkanHelpers/vulkanDevice.h>
#include <vulkanHelpers/vulkanWindow.h>
#include <vulkanHelpers/vulkanPipeline.h>
#include <vulkanHelpers/vulkanBuffer.h>
#include <vulkanHelpers/vulkanTexture.h>
#include <vector>

class windowHandler : public Singleton<windowHandler>
{
public:

    static constexpr size_t flagDebugPrint      { 0b0001 };
    static constexpr size_t flagDebugLayer      { 0b0010 };
    static constexpr size_t flagRenderDocLayer  { 0b0100 };

    bool OK() const noexcept;

    ~windowHandler();

    /// @brief process windows messages, you will need to update individual 
    ///        window input updates
    /// @return whether or not the loop should continue
    bool processInputEvents();

    [[nodiscard("Don't throw away my window man")]]
    std::unique_ptr<vulkanWindow> createWindow(windowSetup const& Setup);

    // SHADER MODULES

    VkShaderModule createShaderModule(const char* relPath);

    VkShaderModule createShaderModule(std::vector<char> const& code);

    void destroyShaderModule(VkShaderModule& shaderModule);

    // PIPELINE LAYOUTS

    bool setupVertexInputInfo(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup);

    VkPipelineLayout createPipelineLayout(VkPipelineLayoutCreateInfo const& CreateInfo);

    void destroyPipelineLayout(VkPipelineLayout& pipelineLayout);

    // TEXTURES (implementation in vulkanTexture.cpp)

    bool createTexture(vulkanTexture& outTexture, vulkanTexture::Setup const& inSetup);

    void transitionImageLayout(VkImage image, VkFormat format, uint32_t mipLevels, bool isTransferStart);// hacky

    void destroyTexture(vulkanTexture& inTexture);

    // one time submit command buffer

    VkCommandBuffer beginOneTimeSubmitCommand(bool useMainCommandPool = false);

    void endOneTimeSubmitCommand(VkCommandBuffer toEnd, bool useMainCommandPool = false);

    // Buffers

    /// @brief write into a buffer through a staging buffer
    /// @param dstBuffer destination buffer
    /// @param srcData data source
    /// @param srcLen length of the data to write to the buffer
    /// @return true if the write is successful, false otherwise
    bool writeToBuffer(vulkanBuffer& dstBuffer, std::vector<void*> const& srcs, std::vector<VkDeviceSize> const& srcLens);
    bool createBuffer(vulkanBuffer& outBuffer, vulkanBuffer::Setup const& inSetup);
    void destroyBuffer(vulkanBuffer& inBuffer);

private:
    friend class Singleton;
    windowHandler& operator=(windowHandler const&) = delete;
    windowHandler(windowHandler const&) = delete;
private:

    windowHandler(size_t flagOptions);

    /// @brief copy from a staging buffer to another buffer
    /// @param dstBuffer destination buffer (must have destination bit set)
    /// @param srcBuffer source buffer (must have source bit set)
    /// @param cpySize size of data to be copied
    /// @return true if copy successful, false otherwise
    bool copyBuffer(vulkanBuffer& dstBuffer, vulkanBuffer& srcBuffer, VkDeviceSize cpySize);

    std::shared_ptr<vulkanInstance> m_pVKInst;  // shared so stuff can depend on it
    std::shared_ptr<vulkanDevice> m_pVKDevice;  // has a copy of m_pVKInst

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield bDebugPrint : 1;   // does not affect error/warning printouts

};

#endif//GRAPHICS_HANDLER_VULKAN_HEADER
