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

#pragma warning (disable : 26451)
#include <assimp/Importer.hpp>  // file IO
#include <assimp/scene.h>       // output data
#include <assimp/postprocess.h> // importer flags
#pragma warning (default : 26451)

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

bool vulkanModel::load3DUVModel(std::string_view const& fPath)
{
  assert(m_Buffer_Vertex.m_Buffer == VK_NULL_HANDLE && m_Buffer_Index.m_Buffer == VK_NULL_HANDLE);
  windowHandler* pWH{ windowHandler::getPInstance() };
  assert(pWH != nullptr);// debug only, flow should be pretty standard.

  Assimp::Importer Importer;
  aiScene const* pScene{ Importer.ReadFile(fPath.data(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices) };
  if (pScene == nullptr || false == pScene->HasMeshes())return false;

  std::vector<VTX_3D_UV> vertices;
  std::vector<uint16_t> indices;

  {
    aiMesh& refMesh{ *pScene->mMeshes[0] };
    if (false == refMesh.HasTextureCoords(0))
    {
      printWarning('\"' + std::string{fPath} + "\" has no/incomplete texture coordinates", true);
      return false;
    }

    // Set up vertices
    vertices.reserve(refMesh.mNumVertices);
    for (unsigned int i{ 0 }, t{ refMesh.mNumVertices }; i < t; ++i)
    {
      aiVector3D& refVtx{ refMesh.mVertices[i] };
      aiVector3D& refUV{ refMesh.mTextureCoords[0][i] };// I hope 0 is good
      vertices.emplace_back
      (
        VTX_3D_UV
        {
          decltype(VTX_3D_UV::m_Pos){ refVtx.x, refVtx.y, refVtx.z },
          decltype(VTX_3D_UV::m_Tex){ refUV.x, refUV.y }
        }
      );
    }

    // Set up Indices
    if (refMesh.HasFaces())
    {
      indices.reserve(refMesh.mNumFaces * 3);// assume all faces are tris
      for (unsigned int i{ 0 }, t{ refMesh.mNumFaces }; i < t; ++i)
      {
        aiFace& refFace{ refMesh.mFaces[i] };
        for (unsigned int j{ 0 }; j < refFace.mNumIndices; ++j)
        {
          indices.emplace_back(static_cast<decltype(indices)::value_type>(refFace.mIndices[j]));
        }
      }
    }
  }

  m_VertexCount = static_cast<uint32_t>(vertices.size());
  m_IndexCount = static_cast<uint32_t>(indices.size());

  // in case I ever want to change or copy it somewhere
  if constexpr (std::is_same_v<decltype(indices)::value_type, uint8_t>)m_IndexType = VK_INDEX_TYPE_UINT8_EXT;
  if constexpr (std::is_same_v<decltype(indices)::value_type, uint16_t>)m_IndexType = VK_INDEX_TYPE_UINT16;
  if constexpr (std::is_same_v<decltype(indices)::value_type, uint32_t>)m_IndexType = VK_INDEX_TYPE_UINT32;

  // Set up vertex buffer
  if (false == pWH->createBuffer
  (
    m_Buffer_Vertex,
    {
      .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Vertex },
      .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Vertex },
      .m_Count{ static_cast<uint32_t>(vertices.size()) },
      .m_ElemSize{ sizeof(decltype(vertices)::value_type) }
    }
  ))
  {
    printWarning("failed to create model vertex buffer"sv, true);
    return false;
  }

  // write vertex buffer
  pWH->writeToBuffer
  (
    m_Buffer_Vertex,
    {
      vertices.data()
    },
    {
      vertices.size() * sizeof(decltype(vertices)::value_type)
    }
  );

  // Set up index buffer
  if (m_IndexCount)
  {
    if (false == pWH->createBuffer
    (
      m_Buffer_Index,
      {
        .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Index },
        .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Index },
        .m_Count{ m_IndexCount },
        .m_ElemSize{ sizeof(uint16_t) }
      }
    ))
    {
      printWarning("failed to create model index buffer"sv, true);
      return false;
    }

    // write index buffer
    pWH->writeToBuffer
    (
      m_Buffer_Index,
      {
        indices.data()
      },
      {
        indices.size() * sizeof(uint16_t)
      }
    );
  }
  else
  {
    m_Buffer_Index = vulkanBuffer{  };
  }

  return true;
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
