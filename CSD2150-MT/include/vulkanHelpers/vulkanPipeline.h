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
#include <string_view>
#include <array>
#include <vulkanHelpers/vulkanBuffer.h>
#include <vulkanHelpers/uniformBuffers.h>

struct vulkanPipeline
{
  template <typename... Ts>
  static inline constexpr uint32_t totalTypesSize() noexcept { return (0 + ... + sizeof(Ts)); }

  template <typename... pushConstantTypes>
  static inline constexpr VkPushConstantRange createPushConstantInfo(VkShaderStageFlags targetStage) noexcept
  {
    return VkPushConstantRange
    {
      .stageFlags{ targetStage },
      .offset{ 0 },
      .size{ totalTypesSize<pushConstantTypes...>() }
    };
  }

  struct uniformInfo
  {
    uint32_t         m_TypeBindingID; // where it binds to
    uint32_t         m_TypeSize;      // size to allocate buffer
    uint32_t         m_TypeAlign;     // unused but might be useful
    VkDescriptorType m_DescriptorType;// data/sampler
  };

  template <typename... Ts>
  static inline constexpr std::vector<uniformInfo> createUniformInfo(uint32_t firstBindingID = 0)
  {
    return std::vector<uniformInfo>
    { uniformInfo{
      .m_TypeBindingID  { firstBindingID++ }, // keep incrementing bind ID
      .m_TypeSize       { sizeof(Ts) },       // variadic unpacked size
      .m_TypeAlign      { alignof(Ts) },      // variadic unpacked alignment
      .m_DescriptorType { /*std::is_same_v<Ts, samplerType>*/ }
    }... };
  }

  enum class E_VERTEX_BINDING_MODE
  {
    UNDEFINED,

    AOS_XY_UV_F32,
    AOS_XY_RGB_F32,
    AOS_XY_RGBA_F32,
    AOS_XYZ_UV_F32,
    AOS_XYZ_RGB_F32,
    AOS_XYZ_RGBA_F32,
    
    // maybe support SOA next time
    //SOA_XY_RGB_F32,
    //SOA_XY_RGBA_F32,
    //SOA_XYZ_RGB_F32,
    //SOA_XYZ_RGBA_F32
  };

  struct setup
  {
    // vertex input data
    E_VERTEX_BINDING_MODE m_VertexBindingMode{ E_VERTEX_BINDING_MODE::UNDEFINED };

    // shader paths
    std::string_view m_PathShaderVert{  };
    std::string_view m_PathShaderFrag{  };

    // hack because why can't anyone be straight with where descriptor sets go
    decltype(fixedUniformBuffers::m_DescriptorSetLayouts) const* m_pSetLayouts{ nullptr };

    // will be used directly for pPushConstantRanges, don't move it around.
    VkPushConstantRange m_PushConstantRangeVert{ createPushConstantInfo<>(VK_SHADER_STAGE_VERTEX_BIT) };
    VkPushConstantRange m_PushConstantRangeFrag{ createPushConstantInfo<>(VK_SHADER_STAGE_FRAGMENT_BIT) };
  };

  // moved in, dangerous... keep track of allocations maybe?
  VkShaderModule                                    m_ShaderVert          { VK_NULL_HANDLE };
  VkShaderModule                                    m_ShaderFrag          { VK_NULL_HANDLE };
  VkPipelineLayout                                  m_PipelineLayout      { VK_NULL_HANDLE };

  // array of 2, 1 for vertex shader, 1 for fragment shader.
  // vectors for each frame in flight.
  std::array<VkDescriptorSetLayout, 2>              m_DescriptorSetLayouts;
  std::array<std::vector<VkDescriptorSet>, 2>       m_DescriptorSets;
  std::array<std::vector<vulkanBuffer>, 2>          m_DescriptorBuffers;

  std::array<VkPipelineShaderStageCreateInfo, 2>    m_ShaderStages        {};
  std::array<VkVertexInputBindingDescription, 1>    m_BindingDescription  {};
  std::array<VkVertexInputAttributeDescription, 2>  m_AttributeDescription{};
  VkPipelineVertexInputStateCreateInfo              m_VertexInputInfo     {};
  VkPipelineInputAssemblyStateCreateInfo            m_InputAssembly       {};
  // viewport will be taken from window when applying this pipeline
  // scissor too
  // pipelineviewportstatecreateinfo made on the spot when time comes
  VkPipelineRasterizationStateCreateInfo            m_Rasterizer          {};
  VkPipelineMultisampleStateCreateInfo              m_Multisampling       {};
  VkPipelineDepthStencilStateCreateInfo             m_DepthStencilState   {};
  VkPipelineColorBlendAttachmentState               m_ColorBlendAttachment{};
  VkPipelineColorBlendStateCreateInfo               m_ColorBlending       {};
  std::array<VkDynamicState, 2>                     m_DynamicStates       {};
  VkPipelineDynamicStateCreateInfo                  m_DynamicStateCreateInfo{};
};

#endif//VULKAN_PIPELINE_HELPER_HEADER
