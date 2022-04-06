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

  glm::mat4 m_LookMat;
  glm::mat4 m_W2V;

  glm::ivec2 m_CursorPrev;

  static constexpr glm::vec2 s_DistLimits{ 250.0f, 500.0f };
  static constexpr float s_ScrollSpeedMul{ 2.0f };
  static constexpr float s_CamFOV{ glm::radians(75.0f) };
  static constexpr float s_Near{ 0.125f };
  static constexpr float s_Far{ s_DistLimits.y * 1.5f };
  static const float s_RotYMin;
  static const float s_RotYMax;
  static const glm::vec3 s_Tgt;
  static const glm::vec3 s_Up;
};

// nextafter only constexpr after C++23
const float originCamera::s_RotYMin{ std::nextafterf(-glm::half_pi<float>(), 0.0f) };
const float originCamera::s_RotYMax{ std::nextafterf( glm::half_pi<float>(), 0.0f) };
const glm::vec3 originCamera::s_Tgt{ 0.0f, 0.0f, 0.0f };
const glm::vec3 originCamera::s_Up{ 0.0f, 1.0f, 0.0f };

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

    vulkanModel exampleModel;
    if (false == exampleModel.load3DUVModel("../Assets/Meshes/Skull_textured.fbx"))
    {
      printWarning("Failed to load example model"sv, true);
      return -4;
    }

    vulkanPipeline trianglePipeline;
    if (!pWH->createPipelineInfo(trianglePipeline,
      vulkanPipeline::setup
      {
        .m_VertexBindingMode{ vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_UV_F32 },
        
        .m_PathShaderVert{ "../Assets/Shaders/Vert.spv"sv },
        .m_PathShaderFrag{ "../Assets/Shaders/Frag.spv"sv },

        // TODO: move createPipelineInfo to be window specific?
        .m_pSetLayouts{ &upVKWin->m_UniformBuffers.m_DescriptorSetLayouts },

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

      float AR{ static_cast<float>(upVKWin->m_windowsWindow.getWidth()) / upVKWin->m_windowsWindow.getHeight() };

      static originCamera cam
      {
        .m_Dist       { originCamera::s_DistLimits.x },
        .m_Rot        { 0.0f, 0.0f },
        .m_LookMat
        {
          glm::lookAt(glm::vec3{ cam.m_Dist, 0.0f, 0.0f }, cam.s_Tgt, cam.s_Up)
        },
        .m_W2V
        {
          glm::perspective(originCamera::s_CamFOV, AR, originCamera::s_Near, originCamera::s_Far)*
          cam.m_LookMat
        }
      };

      glm::ivec2 cursorCurr;
      win0Input.getCursorPos(cursorCurr.x, cursorCurr.y);
      if (int scroll{ win0Input.getScrollSteps() }; scroll || win0Input.isPressed(VK_RBUTTON))
      {
        // update cam rotation based on cursor delta (speed based on pixels)
        glm::vec2 cursorDelta{ (cursorCurr - cam.m_CursorPrev) };
        cursorDelta *= 0.0078125f;
        cam.m_Rot.x -= cursorDelta.x;
        cam.m_Rot.y = glm::clamp(cam.m_Rot.y + cursorDelta.y, cam.s_RotYMin, cam.s_RotYMax);
        
        // update cam distance from origin based on scroll
        cam.m_Dist = glm::clamp(cam.m_Dist - cam.s_ScrollSpeedMul * scroll, cam.s_DistLimits.x, cam.s_DistLimits.y);

        // update lookat
        glm::vec3 camPos
        {
          MTU::axisAngleRotation(cam.s_Up, cam.m_Rot.x, nullptr) *
          glm::vec3{ cosf(cam.m_Rot.y), sinf(cam.m_Rot.y), 0.0f} * cam.m_Dist
        };

        cam.m_LookMat = glm::lookAt(camPos, cam.s_Tgt, cam.s_Up);

        // update W2V matrix
        cam.m_W2V = 
        (
          glm::perspective(originCamera::s_CamFOV, AR, originCamera::s_Near, originCamera::s_Far) *
          cam.m_LookMat
        );
      }
      cam.m_CursorPrev = cursorCurr;

      // ******************************************** CAMERA UPDATE END ****
      // *******************************************************************

      // FCB stands for Frame Command Buffer, this frame's command buffer!
      if (VkCommandBuffer FCB{ upVKWin->FrameBegin() }; FCB != VK_NULL_HANDLE)
      {
      // *******************************************************************
      // ******************************************** RENDER LOOP BEGIN ****
        upVKWin->createAndSetPipeline(trianglePipeline);

        // uniform test here
        {
          uniformVert tmpFUV{ 0.0f };
          upVKWin->updateFixedUniformBuffer(fixedUniformBuffers::e_vert, &tmpFUV, sizeof(tmpFUV));
        }

        { // Rotating object
          static MTU::Timer lazyTimer{ MTU::Timer::getCurrentTP() };// force start the static timer since I'm lazy
          lazyTimer.stop();// lap the timer, doesn't actually stop it
          float boxRot{ static_cast<float>(lazyTimer.getElapsedCount()) / MTU::Timer::clockFrequency };// ~6s/Rotation
          //glm::mat3 tmpform{ MTU::axisAngleRotation(cam.s_Up, boxRot, nullptr) };
          glm::mat4 xform/**/{ cam.m_W2V };/**/
          //{
          //  glm::vec4{ tmpform[0], 0.0f },
          //  glm::vec4{ tmpform[1], 0.0f },
          //  glm::vec4{ tmpform[2], 0.0f },
          //  glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
          //};
          //xform = cam.m_W2V * xform;
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
