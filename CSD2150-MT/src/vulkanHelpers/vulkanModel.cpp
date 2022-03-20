/*!*****************************************************************************
 * @file    vulkanModel.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    08 MAR 2022
 * @brief   This is the interface for the vulkanModel class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/vulkanModel.h>
#include <handlers/windowHandler.h>

// *****************************************************************************
// ****************************************************** non-class helpers ****



// *****************************************************************************
// ******************************************************* Public functions ****

void vulkanModel::drawVerts(VkCommandBuffer FCB)
{
  VkDeviceSize offsets[]{ 0 };
  vkCmdBindVertexBuffers(FCB, 0, 1, &m_Buffer_Vertex.m_Buffer, offsets);
  vkCmdDraw(FCB, m_VertexCount, 1, 0, 0);
}

void vulkanModel::drawIndexed(VkCommandBuffer FCB)
{
  VkDeviceSize offsets[]{ 0 };
  vkCmdBindVertexBuffers(FCB, 0, 1, &m_Buffer_Vertex.m_Buffer, offsets);
  vkCmdBindIndexBuffer(FCB, m_Buffer_Index.m_Buffer, 0, m_IndexType);
  vkCmdDrawIndexed(FCB, m_IndexCount, 1, 0, 0, 0);
}

void vulkanModel::drawInit(VkCommandBuffer FCB)
{
  m_pFnDraw = ((m_IndexType == VK_INDEX_TYPE_NONE_KHR || m_IndexType == VK_INDEX_TYPE_MAX_ENUM || m_IndexCount == 0) ? &vulkanModel::drawVerts : &vulkanModel::drawIndexed);
  if (FCB != nullptr)(this->*m_pFnDraw)(FCB);
}

void vulkanModel::draw(VkCommandBuffer FCB)
{
  (this->*m_pFnDraw)(FCB);
}

void vulkanModel::destroyModel()
{
  if (windowHandler* pWH{ windowHandler::getPInstance() }; pWH != nullptr)
  {
    pWH->destroyBuffer(m_Buffer_Vertex);
    pWH->destroyBuffer(m_Buffer_Index);
  }
}

// *****************************************************************************
// ****************************************************** Private functions ****



// *****************************************************************************
