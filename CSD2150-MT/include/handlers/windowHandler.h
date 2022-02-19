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
#include <utility/Singleton.h>
#include <vulkanHelpers/vulkanInstance.h>
#include <vulkanHelpers/vulkanDevice.h>
#include <vulkanHelpers/vulkanWindow.h>
#include <vulkanHelpers/vulkanPipeline.h>
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

    VkPipelineLayout createPipelineLayout(VkPipelineLayoutCreateInfo const& CreateInfo);

    void destroyPipelineLayout(VkPipelineLayout& pipelineLayout);

    // PIPELINE INFO (CUSTOM)

    bool createPipelineInfo(vulkanPipeline& outPipeline, VkShaderModule const& vertShader, VkShaderModule const& fragShader);

    // DESCRIPTOR SETS

    VkDescriptorSet createDescriptorSet(VkDescriptorSetAllocateInfo const& CreateInfo, VkDescriptorBufferInfo const& ConfigInfo);

private:
    friend class Singleton;
    windowHandler& operator=(windowHandler const&) = delete;
    windowHandler(windowHandler const&) = delete;
private:

    windowHandler(size_t flagOptions);

    std::shared_ptr<vulkanInstance> m_pVKInst;  // shared so stuff can depend on it
    std::shared_ptr<vulkanDevice> m_pVKDevice;  // has a copy of m_pVKInst

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield bDebugPrint : 1;   // does not affect error/warning printouts

};

#endif//GRAPHICS_HANDLER_VULKAN_HEADER
