/*!*****************************************************************************
 * @file    vulkanBuffer.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    08 MAR 2022
 * @brief   This is the interface for the vulkanBuffer struct
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_BUFFER_HELPER_HEADER
#define VULKAN_BUFFER_HELPER_HEADER

#include <vulkan/vulkan.h>

struct vulkanBuffer
{

  static constexpr VkFlags s_BufferUsage_Staging{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
  static constexpr VkFlags s_MemPropFlag_Staging{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

  static constexpr VkFlags s_BufferUsage_Vertex{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };
  static constexpr VkFlags s_MemPropFlag_Vertex{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

  static constexpr VkFlags s_BufferUsage_Index{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT };
  static constexpr VkFlags s_MemPropFlag_Index{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

  static constexpr VkFlags s_BufferUsage_Uniform{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
  static constexpr VkFlags s_MemPropFlag_Uniform{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

  struct Setup
  {
    VkFlags       m_BufferUsage {   };
    VkFlags       m_MemPropFlag {   };
    uint32_t      m_Count       { 0 };
    uint32_t      m_ElemSize    { 0 };
  };

  Setup           m_Settings    {  };
  VkBuffer        m_Buffer      { VK_NULL_HANDLE };
  VkDeviceMemory  m_BufferMemory{ VK_NULL_HANDLE };
};

#endif//VULKAN_BUFFER_HELPER_HEADER
