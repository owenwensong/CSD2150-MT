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

#include <string_view>
#include <vulkan/vulkan.h>
#include <utility/vertices.h>
#include <vulkanHelpers/vulkanBuffer.h>

struct vulkanModel
{
  vulkanBuffer  m_Buffer_Vertex;
  vulkanBuffer  m_Buffer_Index;
  VkIndexType   m_IndexType { VK_INDEX_TYPE_NONE_KHR };
  uint32_t      m_VertexCount{ 0 };
  uint32_t      m_IndexCount{ 0 };

  void drawVerts(VkCommandBuffer FCB);  // draw by vertex buffer only
  void drawIndexed(VkCommandBuffer FCB);// draw by indexed vertices
  void drawInit(VkCommandBuffer FCB);   // initialize which draw fn to use
  void draw(VkCommandBuffer FCB);       // the draw interface
  void (vulkanModel::* m_pFnDraw)(VkCommandBuffer) { &vulkanModel::drawInit };

  bool load3DUVModel(std::string_view const&);
  void destroyModel();

};

#endif//VULKAN_MODEL_HELPER_HEADER
