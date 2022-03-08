/*!*****************************************************************************
 * @file    vulkanModel.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    08 MAR 2022
 * @brief   This is the interface for the vulkanModel struct
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_MODEL_HELPER_HEADER
#define VULKAN_MODEL_HELPER_HEADER

#include <vulkan/vulkan.h>
#include <utility/vertices.h>
#include <vulkanHelpers/vulkanBuffer.h>

struct vulkanModel
{
  vulkanBuffer  m_Buffer_Vertex;
  vulkanBuffer  m_Buffer_Index;
  VkIndexType   m_IndexType { VK_INDEX_TYPE_NONE_KHR };
  uint32_t      m_IndexCount{ 0 };

  void draw(VkCommandBuffer FCB);
  void destroyModel();

};

#endif//VULKAN_MODEL_HELPER_HEADER
