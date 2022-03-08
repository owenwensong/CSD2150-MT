/*!*****************************************************************************
 * @file    main.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    09 FEB 2022
 * @brief   This is the entry point of the program.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <memory>
#include <handlers/windowHandler.h>
#include <vulkanHelpers/vulkanModel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <utility/matrixTransforms.h>

vulkanModel createHW2Model()
{
  vulkanModel retval{ .m_IndexType{ VK_INDEX_TYPE_UINT16 } };
  if (windowHandler* pWH{ windowHandler::getPInstance() }; pWH != nullptr)
  {
    std::array vertices
    {
      VTX_3D_RGB{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
      VTX_3D_RGB{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
      VTX_3D_RGB{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
      VTX_3D_RGB{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
      VTX_3D_RGB{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f } },
      VTX_3D_RGB{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f } },
      VTX_3D_RGB{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
      VTX_3D_RGB{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
    };

    std::array<uint16_t, 36> indices
    {
      0, 1, 3, 3, 1, 2,
      1, 5, 2, 2, 5, 6,
      5, 4, 6, 6, 4, 7,
      4, 0, 7, 7, 0, 3, 
      3, 2, 7, 7, 2, 6,
      4, 5, 0, 0, 5, 1
    };

    retval.m_IndexCount = static_cast<uint32_t>(indices.size());

    if (false == pWH->createBuffer
    (
      retval.m_Buffer_Vertex, 
      {
        .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Vertex }, 
        .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Vertex }, 
        .m_Count{ static_cast<uint32_t>(vertices.size()) }, 
        .m_ElemSize{ sizeof(decltype(vertices)::value_type) } 
      }
    ))
    {
      printWarning("failed to create model vertex buffer"sv, true);
      return retval;
    }
    if (false == pWH->createBuffer
    (
      retval.m_Buffer_Index,
      {
        .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Index },
        .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Index },
        .m_Count{ retval.m_IndexCount },
        .m_ElemSize{ sizeof(decltype(indices)::value_type) }
      }
    ))
    {
      printWarning("failed to create model index buffer"sv, true);
      return retval;
    }

    pWH->writeToBuffer
    (
      retval.m_Buffer_Vertex, 
      vertices.data(), 
      vertices.size() * sizeof(decltype(vertices)::value_type)
    );
    pWH->writeToBuffer
    (
      retval.m_Buffer_Index,
      indices.data(),
      indices.size() * sizeof(decltype(indices)::value_type)
    );
  }
  return retval;
}

int main()
{
#if defined(DEBUG) || defined(_DEBUG)
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//DEBUG || 
  //char* leak{ new char[69] };

  windowHandler* pWH
  {
    windowHandler::createInstance
    (
      windowHandler::flagDebugPrint |
      windowHandler::flagDebugLayer |
      windowHandler::flagRenderDocLayer
    )
  };
  if (pWH == nullptr || !pWH->OK())
  {
    printWarning("FAILED TO CREATE THE WINDOW HANDLER"sv, true);
    return -3;
  }

  if (std::unique_ptr<vulkanWindow> upVKWin{ pWH->createWindow(windowSetup{.m_ClearColorR{ 0.0f }, .m_ClearColorG{ 0.0f }, .m_ClearColorB{ 0.0f }, .m_Title{ L"TestWindow"sv } }) }; upVKWin && upVKWin->OK())
  {
    windowsInput& win0Input{ upVKWin->m_windowsWindow.m_windowInputs };

    vulkanModel exampleModel{ createHW2Model() };

    vulkanPipeline trianglePipeline;
    if (!pWH->createPipelineInfo(trianglePipeline,
      vulkanPipeline::setup
      {
        .m_VertexBindingMode{ vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_RGB_F32 },
        .m_PathShaderVert{ "../Assets/Shaders/triangleVert.spv"sv },
        .m_PathShaderFrag{ "../Assets/Shaders/triangleFrag.spv"sv },

        // uniform stuff here

        .m_PushConstantRangeVert{ vulkanPipeline::createPushConstantInfo<glm::mat4>(VK_SHADER_STAGE_VERTEX_BIT) },
        .m_PushConstantRangeFrag{ vulkanPipeline::createPushConstantInfo<>(VK_SHADER_STAGE_FRAGMENT_BIT) },
      }))
    {
      printWarning("pipeline prep failed"sv, true);
    }
    else while (pWH->processInputEvents())
    {
      // ***********************************************************************
      // ************************************************ WINDOW LOOP BEGIN ****
      win0Input.update();
      win0Input.debugPrint(0b0101);

      if (win0Input.isTriggered(VK_F11))upVKWin->toggleFullscreen();

      // update camera stuff here?

      // 

      // FCB stands for Frame Command Buffer, this frame's command buffer!
      if (VkCommandBuffer FCB{ upVKWin->FrameBegin() }; FCB != VK_NULL_HANDLE)
      {
      // *******************************************************************
      // ******************************************** RENDER LOOP BEGIN ****
        upVKWin->createAndSetPipeline(trianglePipeline);

        static int counter{ 0 };
        float rot{ static_cast<float>(++counter) / 1000 };
        glm::mat3 tmpform{ MTU::axisAngleRotation({0.0f, 1.0f, 0.0f}, rot, nullptr) };
        glm::mat4 xform
        {
          glm::vec4{ tmpform[0], 0.0f },
          glm::vec4{ tmpform[1], 0.0f },
          glm::vec4{ tmpform[2], 0.0f },
          glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
        };
        vkCmdPushConstants(FCB, trianglePipeline.m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(glm::mat4)), &xform);
        exampleModel.draw(FCB);

        upVKWin->FrameEnd();
        upVKWin->PageFlip();
        // ******************************************** RENDER LOOP END ****
        // *****************************************************************
      }
      // ************************************************** WINDOW LOOP END ****
      // ***********************************************************************
    }
    exampleModel.destroyModel();
    pWH->destroyPipelineInfo(trianglePipeline);
  }

  return 0;

}
