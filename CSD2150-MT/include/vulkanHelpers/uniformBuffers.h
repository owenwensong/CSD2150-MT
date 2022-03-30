/*!*****************************************************************************
 * @file    uniformBuffers.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    30 MAR 2022
 * @brief   This is the declarations of the uniform buffers used
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_UNIFORM_BUFFER_HELPER_HEADER
#define VULKAN_UNIFORM_BUFFER_HELPER_HEADER

#include <vulkanHelpers/vulkanBuffer.h>
#include <vector>
#include <array>

struct uniformVert
{
  float offsetTest;
};

struct uniformFrag
{
  
};

struct fixedUniformBuffers
{

  enum target
  {
    e_vert = 0,
    e_frag,
    e_NUMTARGETS
  };

  static constexpr std::array<uint32_t, e_NUMTARGETS> StructSizes
  {
    sizeof(uniformVert),
    sizeof(uniformFrag)
  };

  static constexpr std::array<vulkanBuffer::Setup, e_NUMTARGETS> BufferSetups
  {
    vulkanBuffer::Setup // Vertex uniforms
    {
      .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Uniform },
      .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Uniform },
      .m_Count      { 1 },
      .m_ElemSize   { sizeof(uniformVert) }
    },
    vulkanBuffer::Setup // Fragment uniforms
    {
      .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Uniform },
      .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Uniform },
      .m_Count      { 1 },
      .m_ElemSize   { sizeof(uniformFrag) }
    }
  };

  static constexpr std::array<VkDescriptorSetLayoutBinding, e_NUMTARGETS> LayoutBindings
  {
    VkDescriptorSetLayoutBinding
    {
      .binding        { 0 },
      .descriptorType { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
      .descriptorCount{ 1 },
      .stageFlags     { VK_SHADER_STAGE_VERTEX_BIT },
      .pImmutableSamplers{ VK_NULL_HANDLE }
    },
    VkDescriptorSetLayoutBinding
    {
      .binding        { 0 },
      .descriptorType { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
      .descriptorCount{ 1 },
      .stageFlags     { VK_SHADER_STAGE_FRAGMENT_BIT },
      .pImmutableSamplers{ VK_NULL_HANDLE }
    }
  };

  // vector of frames
  std::array<VkDescriptorSetLayout, e_NUMTARGETS>         m_DescriptorSetLayouts;
  std::vector<std::array<VkDescriptorSet, e_NUMTARGETS>>  m_DescriptorSets;
  std::vector<std::array<vulkanBuffer, e_NUMTARGETS>>     m_Buffers;
};

#endif//VULKAN_UNIFORM_BUFFER_HELPER_HEADER
