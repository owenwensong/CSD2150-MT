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
  aiScene const* pScene
  {
    Importer.ReadFile
    (
      fPath.data(),
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_PreTransformVertices
    )
  };
  if (pScene == nullptr || false == pScene->HasMeshes())return false;

  std::vector<VTX_3D_UV> vertices;
  std::vector<uint32_t> indices;

  { // reserve all the space needed...
    size_t vSpace{ 0 }, iSpace{ 0 };
    for (unsigned int i{ 0 }, t{ pScene->mNumMeshes }; i < t; ++i)
    {
      vSpace += pScene->mMeshes[i]->mNumVertices;
      for (unsigned int j{ 0 }, k{ pScene->mMeshes[i]->mNumFaces }; j < k; ++j)
      {
        iSpace += pScene->mMeshes[i]->mFaces[j].mNumIndices;
      }
    }
    vertices.reserve(vSpace);
    indices.reserve(iSpace);
  }

  // end up being unnecessary, pretransformvertices was what I needed...
  for (unsigned int i{ 0 }, t{ pScene->mNumMeshes }; i < t; ++i)
  {
    aiMesh& refMesh{ *pScene->mMeshes[i] };
    if (false == refMesh.HasTextureCoords(0))
    {
      printWarning('\"' + std::string{fPath} + "\" has no/incomplete texture coordinates", true);
      return false;
    }

    // save index offset (indices start from last vertex for multi mesh objects)
    size_t indexOffset{ vertices.size() };

    // Set up vertices
    for (unsigned int j{ 0 }, k{ refMesh.mNumVertices }; j < k; ++j)
    {
      aiVector3D& refVtx{ refMesh.mVertices[j] };
      aiVector3D& refUV{ refMesh.mTextureCoords[0][j] };// I hope 0 is good
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
      for (unsigned int j{ 0 }; j < refMesh.mNumFaces; ++j)
      {
        aiFace& refFace{ refMesh.mFaces[j] };
        for (unsigned int k{ 0 }; k < refFace.mNumIndices; ++k)
        {
          indices.emplace_back(static_cast<decltype(indices)::value_type>(indexOffset + refFace.mIndices[k]));
        }
      }
    }// else add by raw vertex?
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
      .m_Count      { m_VertexCount },
      .m_ElemSize   { sizeof(decltype(vertices)::value_type) }
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
        .m_Count      { m_IndexCount },
        .m_ElemSize   { sizeof(decltype(indices)::value_type) }
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
        indices.size() * sizeof(decltype(indices)::value_type)
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
