/*!*****************************************************************************
 * @file    vulkanPipeline.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    19 FEB 2022
 * @brief   This is the interface of the vulkan pipeline struct
 *          this struct holds creation info for the actual pipeline.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_PIPELINE_HELPER_HEADER
#define VULKAN_PIPELINE_HELPER_HEADER

#include <vulkan/vulkan.h>
#include <array>

struct vulkanPipeline
{
    std::array<VkPipelineShaderStageCreateInfo, 2>  m_ShaderStages      {};
    VkPipelineVertexInputStateCreateInfo            m_VertexInputInfo   {};
    VkPipelineInputAssemblyStateCreateInfo          m_InputAssembly     {};
    // viewport will be taken from window when applying this pipeline
    // scissor too
    // pipelineviewportstatecreateinfo made on the spot when time comes
    VkPipelineRasterizationStateCreateInfo          m_Rasterizer        {};
    VkPipelineMultisampleStateCreateInfo            m_Multisampling     {};
    VkPipelineDepthStencilStateCreateInfo           m_DepthStencilState {};
    VkPipelineColorBlendAttachmentState             m_ColorBlendAttachment  {};
    VkPipelineColorBlendStateCreateInfo             m_ColorBlending     {};
    std::array<VkDynamicState, 2>                   m_DynamicStates     {};
    VkPipelineDynamicStateCreateInfo                m_DynamicStateCreateInfo{};
};

#endif//VULKAN_PIPELINE_HELPER_HEADER
