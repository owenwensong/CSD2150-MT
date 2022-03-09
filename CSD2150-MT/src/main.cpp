/*!*****************************************************************************
 * @file    main.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    09 FEB 2022
 * @brief   This is the entry point of the program.
 * 
 *          This file also contains the implementation of Homework 2.
 *          The originCamera settings are at the top for easy adjustments.
 *          Creation of the cube is put into the createHW2Model function.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <memory>
#include <handlers/windowHandler.h>
#include <vulkanHelpers/vulkanModel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <utility/matrixTransforms.h>
#include <utility/Timer.h>

struct originCamera  // struct just for this implementation always facing origin
{
  float     m_Dist;
  glm::vec2 m_Rot;

  glm::mat4 m_W2V;
  glm::mat4 m_LookMat;

  glm::ivec2 m_CursorPrev;

  static constexpr glm::vec2 s_DistLimits{ 2.0f, 50.0f };
  static constexpr float s_CamFOV{ glm::radians(75.0f) };
  static constexpr float s_Near{ 0.125f };
  static constexpr float s_Far{ s_DistLimits.y + 2.0f };
  static const float s_RotYMin;
  static const float s_RotYMax;
  static const glm::vec3 s_Tgt;
  static const glm::vec3 s_Up;
};

// nextafter only constexpr after C++23
const float originCamera::s_RotYMin{ std::nextafterf(-glm::half_pi<float>(), 0.0f) };
const float originCamera::s_RotYMax{ std::nextafterf( glm::half_pi<float>(), 0.0f) };
const glm::vec3 originCamera::s_Tgt{ 0.0f, 0.0f, 0.0f };
const glm::vec3 originCamera::s_Up{ 0.0f, -1.0f, 0.0f };

// wanted to use DPML, but probably not allowed since can't pull it from git
vulkanModel createHW2Model()
{
  vulkanModel retval{ .m_IndexType{ VK_INDEX_TYPE_UINT16 } };
  if (windowHandler* pWH{ windowHandler::getPInstance() }; pWH != nullptr)
  {
    // shared vertices version
    //std::array vertices
    //{
    //  VTX_3D_RGB{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    //  VTX_3D_RGB{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    //  VTX_3D_RGB{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
    //  VTX_3D_RGB{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
    //  VTX_3D_RGB{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f } },
    //  VTX_3D_RGB{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f } },
    //  VTX_3D_RGB{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
    //  VTX_3D_RGB{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
    //};

    //std::array<uint16_t, 36> indices
    //{
    //  0, 1, 3, 3, 1, 2,
    //  1, 5, 2, 2, 5, 6,
    //  5, 4, 6, 6, 4, 7,
    //  4, 0, 7, 7, 0, 3, 
    //  3, 2, 7, 7, 2, 6,
    //  4, 5, 0, 0, 5, 1
    //};

    std::array vertices
    {
      // Front face
      VTX_3D_RGB{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
      VTX_3D_RGB{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
      VTX_3D_RGB{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
      VTX_3D_RGB{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
      // Back face
      VTX_3D_RGB{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 0.5f } },
      VTX_3D_RGB{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 0.5f } },
      VTX_3D_RGB{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 0.5f } },
      VTX_3D_RGB{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 0.5f } },
      // Top face
      VTX_3D_RGB{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
      VTX_3D_RGB{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
      VTX_3D_RGB{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
      VTX_3D_RGB{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
      // Bottom face
      VTX_3D_RGB{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.5f, 0.0f } },
      VTX_3D_RGB{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.5f, 0.0f } },
      VTX_3D_RGB{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.5f, 0.0f } },
      VTX_3D_RGB{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.5f, 0.0f } },
      // Left face
      VTX_3D_RGB{ {  0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.0f } },
      VTX_3D_RGB{ {  0.5f, -0.5f,  0.5f }, { 0.5f, 0.0f, 0.0f } },
      VTX_3D_RGB{ {  0.5f,  0.5f,  0.5f }, { 0.5f, 0.0f, 0.0f } },
      VTX_3D_RGB{ {  0.5f,  0.5f, -0.5f }, { 0.5f, 0.0f, 0.0f } },
      // Right face
      VTX_3D_RGB{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
      VTX_3D_RGB{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
      VTX_3D_RGB{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
      VTX_3D_RGB{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },

    };

    std::array<uint16_t, 36> indices
    {
      0,  1,  2,  2,  3,  0,
      4,  7,  6,  6,  5,  4,
      8,  9,  10, 10, 11, 8,
      12, 15, 14, 14, 13, 12,
      16, 19, 18, 18, 17, 16,
      20, 21, 22, 22, 23, 20
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

  if (std::unique_ptr<vulkanWindow> upVKWin{ pWH->createWindow(windowSetup{.m_ClearColorR{ 0.5f }, .m_ClearColorG{ 0.5f }, .m_ClearColorB{ 0.5f }, .m_Title{ L"CSD2150 Homework 2 | Owen Huang Wensong"sv } }) }; upVKWin && upVKWin->OK())
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
      //win0Input.debugPrint(0b0101);

      if (win0Input.isTriggered(VK_F11))upVKWin->toggleFullscreen();

      // *******************************************************************
      // ****************************************** CAMERA UPDATE BEGIN ****

      static originCamera cam
      {
        .m_Dist       { 2.0f },
        .m_Rot        { 0.0f, 0.0f },
        .m_LookMat    
        {
          glm::lookAt(glm::vec3{ cam.m_Dist, 0.0f, 0.0f }, cam.s_Tgt, cam.s_Up)
        }
      };

      float AR{ static_cast<float>(upVKWin->m_windowsWindow.getWidth()) / upVKWin->m_windowsWindow.getHeight() };

      glm::ivec2 cursorCurr;
      win0Input.getCursorPos(cursorCurr.x, cursorCurr.y);
      if (int scroll{ win0Input.getScrollSteps() }; scroll || win0Input.isPressed(VK_RBUTTON))
      {
        // update cam rotation based on cursor delta (speed based on pixels)
        glm::vec2 cursorDelta{ (cursorCurr - cam.m_CursorPrev) };
        cursorDelta *= 0.0078125f;
        cam.m_Rot.x += cursorDelta.x;
        cam.m_Rot.y = glm::clamp(cam.m_Rot.y + cursorDelta.y, cam.s_RotYMin, cam.s_RotYMax);
        
        // update cam distance from origin based on scroll
        cam.m_Dist = glm::clamp(cam.m_Dist - 0.25f * scroll, cam.s_DistLimits.x, cam.s_DistLimits.y);

        // update lookat
        glm::vec3 camPos
        {
          MTU::axisAngleRotation(cam.s_Up, cam.m_Rot.x, nullptr) *
          glm::vec3{ cosf(cam.m_Rot.y), sinf(cam.m_Rot.y), 0.0f} * cam.m_Dist
        };

        cam.m_LookMat = glm::lookAt(camPos, cam.s_Tgt, cam.s_Up);
      }
      cam.m_CursorPrev = cursorCurr;
      
      // update W2V matrix
      cam.m_W2V = 
      (
        glm::perspective(originCamera::s_CamFOV, AR, originCamera::s_Near, originCamera::s_Far) *
        cam.m_LookMat
      );

      // ******************************************** CAMERA UPDATE END ****
      // *******************************************************************

      // FCB stands for Frame Command Buffer, this frame's command buffer!
      if (VkCommandBuffer FCB{ upVKWin->FrameBegin() }; FCB != VK_NULL_HANDLE)
      {
      // *******************************************************************
      // ******************************************** RENDER LOOP BEGIN ****
        upVKWin->createAndSetPipeline(trianglePipeline);

        { // static hidden box object
          // cam starts with X, so behind is more X
          static const glm::vec3 boxWorldPos{ -1.0f, -0.5f, 0.0f };
          glm::mat4 xform{ glm::translate(cam.m_W2V, boxWorldPos) };
          vkCmdPushConstants(FCB, trianglePipeline.m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(glm::mat4)), &xform);
          exampleModel.draw(FCB);
        }
        { // Rotating box object
          static MTU::Timer lazyTimer{ MTU::Timer::getCurrentTP() };// force start the static timer since I'm lazy
          lazyTimer.stop();// lap the timer, doesn't actually stop it
          float boxRot{ static_cast<float>(lazyTimer.getElapsedCount()) / MTU::Timer::clockFrequency };// ~6s/Rotation
          // axis angle rotation, so facing straight down (relative to up) will get CW 
          glm::mat3 tmpform{ MTU::axisAngleRotation(-cam.s_Up, boxRot, nullptr) };
          glm::mat4 xform
          {
            glm::vec4{ tmpform[0], 0.0f },
            glm::vec4{ tmpform[1], 0.0f },
            glm::vec4{ tmpform[2], 0.0f },
            glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
          };
          xform = cam.m_W2V * xform;
          vkCmdPushConstants(FCB, trianglePipeline.m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(glm::mat4)), &xform);
          exampleModel.draw(FCB);
        }

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
