/*!*****************************************************************************
 * @file    main.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    09 FEB 2022
 * @brief   This is the entry point of the program.
 * 
 *          This is where the final project example lives. The vulkan engine
 *          is separate from this file.
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
  glm::vec3 m_Pos;
  glm::vec2 m_Rot;

  glm::mat4 m_LookMat;
  glm::mat4 m_W2V;

  glm::ivec2 m_CursorPrev;

  static constexpr glm::vec2 s_DistLimits{ 2.5f, 25.0f };
  static constexpr float s_ScrollSpeedMul{ 0.125f };
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
const float originCamera::s_RotYMax{ std::nextafterf(glm::half_pi<float>(), 0.0f) };
const glm::vec3 originCamera::s_Tgt{ 0.0f, 0.0f, 0.0f };
const glm::vec3 originCamera::s_Up{ 0.0f, 1.0f, 0.0f };

struct pointLight // aligned for use in uniform buffer
{
  alignas(16) glm::vec3 m_Pos;
  alignas(16) glm::vec3 m_Col;
};

struct objInfo  // fast for me, slow for the computer (keep it simple, no rotation)
{
  glm::vec3 m_Scale;
  glm::vec3 m_Translation;

  glm::mat4 m_M2W;
  glm::mat4 m_W2M;

  void genMemberMatrices()
  {
    m_M2W = glm::scale(glm::translate(glm::identity<glm::mat4>(), m_Translation), m_Scale);
    m_W2M = glm::inverse(m_M2W);
  }
};

namespace FinalInfos
{
  enum objID
  {
    E_SKULL = 0,
    E_CAR,
    E_NUM_OBJS
  };

  enum viewModes
  {
    E_SKULL_ONLY = 0,
    E_CAR_ONLY,
    E_BOTH,
    E_NUM_VIEWS
  };

  void switchMode(objInfo& toSwitch, size_t whichObj, size_t whichMode)
  {
    switch (whichObj)
    {
    case E_SKULL:
      switch (whichMode)
      {
      case E_SKULL_ONLY:
        toSwitch.m_Scale = glm::vec3{ 0.03125f, 0.03125f, 0.03125f };
        toSwitch.m_Translation = glm::vec3{ 0.0f, 0.0f, 0.0f };
        break;
      case E_CAR_ONLY:
        toSwitch.m_Scale = glm::vec3{ 0.03125f, 0.03125f, 0.03125f };
        toSwitch.m_Translation = glm::vec3{ 0.0f, 0.0f, -100.0f };
        break;
      case E_BOTH:
        toSwitch.m_Scale = glm::vec3{ 0.0078125f, 0.0078125f, 0.0078125f };
        toSwitch.m_Translation = glm::vec3{ 1.0f, 3.5f, -4.0f };
        break;
      default:
        return;
      }
      break;
    case E_CAR:
      switch (whichMode)
      {
      case E_SKULL_ONLY:
        toSwitch.m_Scale = glm::vec3{ 2.0f, 2.0f, 2.0f };
        toSwitch.m_Translation = glm::vec3{ 0.0f, 0.0f, -100.0f };
        break;
      case E_CAR_ONLY:
        toSwitch.m_Scale = glm::vec3{ 2.0f, 2.0f, 2.0f };
        toSwitch.m_Translation = glm::vec3{ 0.0f, -2.5f, 0.0f };
        break;
      case E_BOTH:
        toSwitch.m_Scale = glm::vec3{ 2.0f, 2.0f, 2.0f };
        toSwitch.m_Translation = glm::vec3{ 0.0f, -2.5f, 0.0f };
        break;
      default:
        return;
      }
      break;
    default:
      return;
    }
    toSwitch.genMemberMatrices();
  }

}

namespace FinalSkull  // helper hard coded stuff
{
  enum texID
  {
    E_BASE_COLOR = 0,
    E_AMBIENT_OCCLUSION,
    E_NORMAL,
    E_ROUGHNESS,
    E_NUM_TEXTURES
  };

  static const char* texPaths[E_NUM_TEXTURES]
  {
    "../Assets/Textures/Skull/TD_Checker_Base_Color.dds",
    "../Assets/Textures/Skull/TD_Checker_Mixed_AO.dds",
    "../Assets/Textures/Skull/TD_Checker_Normal_OpenGL.dds",
    "../Assets/Textures/Skull/TD_Checker_Roughness.dds"
  };

  static bool loadTextures(std::array<vulkanTexture, E_NUM_TEXTURES>& outTextures)
  {
    windowHandler* pWH{ windowHandler::getPInstance() };
    assert(pWH);

    for (size_t i{ 0 }, t{ E_NUM_TEXTURES }; i < t; ++i)
    {
      if (false == pWH->createTexture(outTextures[i], vulkanTexture::Setup{ .m_Path{ texPaths[i] } }))
      {
        return false;
      }
    }
    return true;
  }

  static void unloadTextures(std::array<vulkanTexture, E_NUM_TEXTURES>& toClear)
  {
    windowHandler* pWH{ windowHandler::getPInstance() };
    assert(pWH);

    for (vulkanTexture& x : toClear)pWH->destroyTexture(x);
  }
}

namespace FinalCar
{
  enum texID
  {
    E_BASE_COLOR = 0,
    E_AMBIENT_OCCLUSION,
    E_NORMAL,
    E_ROUGHNESS,
    E_NUM_TEXTURES
  };

  static const char* texPaths[E_NUM_TEXTURES]
  {
    "../Assets/Textures/VintageCar/_Base_Color.dds",
    "../Assets/Textures/VintageCar/_Mixed_AO.dds",
    "../Assets/Textures/VintageCar/_Normal_DirectX.dds",
    "../Assets/Textures/VintageCar/_Roughness.dds"
  };

  static bool loadTextures(std::array<vulkanTexture, E_NUM_TEXTURES>& outTextures)
  {
    windowHandler* pWH{ windowHandler::getPInstance() };
    assert(pWH);

    for (size_t i{ 0 }, t{ E_NUM_TEXTURES }; i < t; ++i)
    {
      if (false == pWH->createTexture(outTextures[i], vulkanTexture::Setup{ .m_Path{ texPaths[i] } }))
      {
        return false;
      }
    }
    return true;
  }

  static void unloadTextures(std::array<vulkanTexture, E_NUM_TEXTURES>& toClear)
  {
    windowHandler* pWH{ windowHandler::getPInstance() };
    assert(pWH);

    for (vulkanTexture& x : toClear)pWH->destroyTexture(x);
  }
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

  std::array<vulkanTexture, FinalSkull::E_NUM_TEXTURES> SkullTextures;
  if (false == FinalSkull::loadTextures(SkullTextures))
  {
    printWarning("Failed to load skull texture(s)"sv, true);
    return -4;
  }
  std::array<vulkanTexture, FinalCar::E_NUM_TEXTURES> CarTextures;
  if (false == FinalCar::loadTextures(CarTextures))
  {
    printWarning("Failed to load car texture(s)"sv, true);
    return -4;
  }

  printf_s
  (
    "INSTRUCTIONS:\n"
    "3 Different view modes exist, switched using the number row keys.\n"
    "1: SKULL ONLY\n"
    "2: CAR ONLY\n"
    "3: BOTH (Skull will be in the seat :D)\n\n"
    "LIGHTING CONTROLS:\n"
    "Spacebar: Toggle between directed light following or leave it behind\n"
    "A + UP/DOWN: Increase/Decrease ambient lighting\n"
    "R + UP/DOWN: Increase/Decrease directed light's Red intensity\n"
    "G + UP/DOWN: Increase/Decrease directed light's Green intensity\n"
    "B + UP/DOWN: Increase/Decrease directed light's Blue intensity\n"
    "+: Increase Gamma (hold shift for quick change)\n"
    "-: Decrease Gamma (hold shift for quick change)\n\n"
    "OTHER CONTROLS:\n"
    "F11: Enter fullscreen mode\n"
  );

  if (std::unique_ptr<vulkanWindow> upVKWin{ pWH->createWindow(windowSetup{.m_ClearColorR{ 0.0f }, .m_ClearColorG{ 0.0f }, .m_ClearColorB{ 0.0f }, .m_Title{ L"CSD2150 Final Project | Owen Huang Wensong"sv } }) }; upVKWin && upVKWin->OK())
  {
    windowsInput& win0Input{ upVKWin->m_windowsWindow.m_windowInputs };

    vulkanModel skullModel;
    if (false == skullModel.load3DUVModel("../Assets/Meshes/Skull_textured.fbx"))
    {
      printWarning("Failed to load skull model"sv, true);
      return -5;
    }
    vulkanModel carModel;
    if (false == carModel.load3DUVModel("../Assets/Meshes/_2_Vintage_Car_01_low.fbx"))
    {
      printWarning("Failed to load car model"sv, true);
      return -5;
    }

    objInfo skullInfo, carInfo;
    FinalInfos::switchMode(skullInfo, FinalInfos::E_SKULL, FinalInfos::E_SKULL_ONLY);
    FinalInfos::switchMode(carInfo, FinalInfos::E_CAR, FinalInfos::E_SKULL_ONLY);

    
    vulkanPipeline skullPipeline, carPipeline;
    if (false == upVKWin->createPipelineInfo(skullPipeline,
      vulkanPipeline::setup // ***************************** SKULL PIPELINE ****
      {
        .m_VertexBindingMode{ vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_UV_NML_TAN_F32 },
        
        .m_PathShaderVert{ "../Assets/Shaders/Vert.spv"sv },
        .m_PathShaderFrag{ "../Assets/Shaders/fragBottomUpNormalsBC5.spv"sv },

        .m_UniformsVert
        {
          vulkanPipeline::createUniformInfo
          <
            //float,        // heightmap scale
            //vulkanTexture // u_sRoughness (for heightmap data)
          >()
        },
        .m_UniformsFrag
        {
          vulkanPipeline::createUniformInfo
          <
            float,        // u_AmbientStrength
            glm::vec3,    // u_LocalCamPos
            pointLight,   // u_LocalLightPos & u_LocalLightCol
            vulkanTexture,// u_sColor
            vulkanTexture,// u_sAmbient
            vulkanTexture,// u_sNormal
            vulkanTexture // u_sRoughness
          >()
        },

        .m_pTexturesVert
        {
          //&SkullTextures[FinalSkull::E_ROUGHNESS]
        },
        .m_pTexturesFrag
        {
          &SkullTextures[FinalSkull::E_BASE_COLOR],
          &SkullTextures[FinalSkull::E_AMBIENT_OCCLUSION],
          &SkullTextures[FinalSkull::E_NORMAL],
          &SkullTextures[FinalSkull::E_ROUGHNESS]
        },

        .m_PushConstantRangeVert{ vulkanPipeline::createPushConstantInfo<glm::mat4>(VK_SHADER_STAGE_VERTEX_BIT) },
        .m_PushConstantRangeFrag{ vulkanPipeline::createPushConstantInfo<float>(VK_SHADER_STAGE_FRAGMENT_BIT) },
      }) || false == upVKWin->createPipelineInfo(carPipeline,
      vulkanPipeline::setup // ******************************* CAR PIPELINE ****
      {
        .m_VertexBindingMode{ vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_UV_NML_TAN_F32 },

        .m_PathShaderVert{ "../Assets/Shaders/Vert.spv"sv },
        .m_PathShaderFrag{ "../Assets/Shaders/fragTopDownNormalslR8G8B8A8.spv"sv },

        .m_UniformsVert
        {
          vulkanPipeline::createUniformInfo<>()
        },
        .m_UniformsFrag
        {
          vulkanPipeline::createUniformInfo
          <
            float,        // u_AmbientStrength
            glm::vec3,    // u_LocalCamPos
            pointLight,   // u_LocalLightPos & u_LocalLightCol
            vulkanTexture,// u_sColor
            vulkanTexture,// u_sAmbient
            vulkanTexture,// u_sNormal
            vulkanTexture // u_sRoughness
          >()
        },

      .m_pTexturesVert{ },
      .m_pTexturesFrag
      {
        &CarTextures[FinalCar::E_BASE_COLOR],
        &CarTextures[FinalCar::E_AMBIENT_OCCLUSION],
        &CarTextures[FinalCar::E_NORMAL],
        &CarTextures[FinalCar::E_ROUGHNESS]
      },

      .m_PushConstantRangeVert{ vulkanPipeline::createPushConstantInfo<glm::mat4>(VK_SHADER_STAGE_VERTEX_BIT) },
      .m_PushConstantRangeFrag{ vulkanPipeline::createPushConstantInfo<float>(VK_SHADER_STAGE_FRAGMENT_BIT) },
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
        .m_Dist       { 0.5f * (originCamera::s_DistLimits.x + originCamera::s_DistLimits.y) },
        .m_Pos        { 0.0f, 0.0f, cam.m_Dist },
        .m_Rot        { -glm::half_pi<float>(), 0.0f},
        .m_LookMat
        {
          glm::lookAt(glm::vec3{ 0.0f, 0.0f, cam.m_Dist }, cam.s_Tgt, cam.s_Up)
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
        cam.m_Pos =
        {
          MTU::axisAngleRotation(cam.s_Up, cam.m_Rot.x, nullptr) *
          glm::vec3{ cosf(cam.m_Rot.y), sinf(cam.m_Rot.y), 0.0f} * cam.m_Dist
        };

        cam.m_LookMat = glm::lookAt(cam.m_Pos, cam.s_Tgt, cam.s_Up);

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
        
        { // Setting uniforms
          //static float skullHeightMapScale{ 1.0f };
          //if (win0Input.isPressed(VK_UP))skullHeightMapScale += 0.5f;
          //if (win0Input.isPressed(VK_DOWN))skullHeightMapScale -= 0.5f;
          //upVKWin->setUniform(trianglePipeline, 0, 0, &skullHeightMapScale, sizeof(skullHeightMapScale));

          if (win0Input.isTriggered(VK_1))
          {
            size_t mode{ FinalInfos::E_SKULL_ONLY };
            FinalInfos::switchMode(skullInfo, FinalInfos::E_SKULL, mode);
            FinalInfos::switchMode(carInfo, FinalInfos::E_CAR, mode);
            printf_s("Switched to SKULL ONLY mode\n");
          }
          else if (win0Input.isTriggered(VK_2))
          {
            size_t mode{ FinalInfos::E_CAR_ONLY };
            FinalInfos::switchMode(skullInfo, FinalInfos::E_SKULL, mode);
            FinalInfos::switchMode(carInfo, FinalInfos::E_CAR, mode);
            printf_s("Switched to CAR ONLY mode\n");
          }
          else if (win0Input.isTriggered(VK_3))
          {
            size_t mode{ FinalInfos::E_BOTH };
            FinalInfos::switchMode(skullInfo, FinalInfos::E_SKULL, mode);
            FinalInfos::switchMode(carInfo, FinalInfos::E_CAR, mode);
            printf_s("Switched to BOTH mode\n");
          }
          

          static float s_AmbientStrength{ 0.0625f };
          static pointLight s_Light
          {
            .m_Pos{ cam.m_Pos },
            .m_Col{ 1.0f, 1.0f, 1.0f }
          };
          static bool s_bLightFollow{ true };
          if (win0Input.isTriggered(VK_SPACE))s_bLightFollow = !s_bLightFollow;
          if (s_bLightFollow)s_Light.m_Pos = cam.m_Pos;
          if (win0Input.isTriggered(VK_UP))
          {
            if (win0Input.isPressed(VK_R) && (s_Light.m_Col.r += 0.125f) > 1.0f)s_Light.m_Col.r = 1.0f;
            if (win0Input.isPressed(VK_G) && (s_Light.m_Col.g += 0.125f) > 1.0f)s_Light.m_Col.g = 1.0f;
            if (win0Input.isPressed(VK_B) && (s_Light.m_Col.b += 0.125f) > 1.0f)s_Light.m_Col.b = 1.0f;
            if (win0Input.isPressed(VK_A) && (s_AmbientStrength += 0.0625f) > 1.0f)s_AmbientStrength = 1.0f;
            printf_s("LIGHT INFO (A/R/G/B + UP/DOWN to adjust):\nAmbient strength: %f\nlightR: %.4f\nlightG: %.4f\nlightB: %.4f\n", s_AmbientStrength, s_Light.m_Col.r, s_Light.m_Col.g, s_Light.m_Col.b);
          }
          else if (win0Input.isTriggered(VK_DOWN))
          {
            if (win0Input.isPressed(VK_R) && (s_Light.m_Col.r -= 0.125f) < 0.0f)s_Light.m_Col.r = 0.0f;
            if (win0Input.isPressed(VK_G) && (s_Light.m_Col.g -= 0.125f) < 0.0f)s_Light.m_Col.g = 0.0f;
            if (win0Input.isPressed(VK_B) && (s_Light.m_Col.b -= 0.125f) < 0.0f)s_Light.m_Col.b = 0.0f;
            if (win0Input.isPressed(VK_A) && (s_AmbientStrength -= 0.0625f) < 0.0f)s_AmbientStrength = 0.0f;
            printf_s("LIGHT INFO (A/R/G/B + UP/DOWN to adjust):\nAmbient strength: %f\nlightR: %.4f\nlightG: %.4f\nlightB: %.4f\n", s_AmbientStrength, s_Light.m_Col.r, s_Light.m_Col.g, s_Light.m_Col.b);
          }

          pointLight l_light{ s_Light };
          glm::vec3 l_camPos{ skullInfo.m_W2M * glm::vec4{ cam.m_Pos, 1.0f } };
          l_light.m_Pos = skullInfo.m_W2M * glm::vec4{ s_Light.m_Pos, 1.0f };
          upVKWin->setUniform(skullPipeline, 1, 0, &s_AmbientStrength, sizeof(s_AmbientStrength));
          upVKWin->setUniform(skullPipeline, 1, 1, &l_camPos, sizeof(l_camPos));
          upVKWin->setUniform(skullPipeline, 1, 2, &l_light, sizeof(l_light));
          
          l_camPos = carInfo.m_W2M * glm::vec4{ cam.m_Pos, 1.0f };
          l_light.m_Pos = carInfo.m_W2M * glm::vec4{ s_Light.m_Pos, 1.0f };
          upVKWin->setUniform(carPipeline, 1, 0, &s_AmbientStrength, sizeof(s_AmbientStrength));
          upVKWin->setUniform(carPipeline, 1, 1, &l_camPos, sizeof(l_camPos));
          upVKWin->setUniform(carPipeline, 1, 2, &l_light, sizeof(l_light));
        }

        static glm::vec2 s_gamma{ 2.25f, 1.0f / 2.25f };
        static constexpr float sc_gammaMin{ 0.125f };
        static constexpr float sc_gammaMax{ 22.5f };
        if (win0Input.isPressed(VK_SHIFT) && win0Input.isPressed(VK_OEM_PLUS) || win0Input.isTriggered(VK_OEM_PLUS))
        {
          if (s_gamma.x != sc_gammaMax)
          {
            s_gamma.x = glm::clamp(s_gamma.x + 0.125f, sc_gammaMin, sc_gammaMax);
            s_gamma.y = 1.0f / s_gamma.x;
            printf_s("Gamma increased: %.3f\n", s_gamma.x);
          }
        }
        else if (win0Input.isPressed(VK_SHIFT) && win0Input.isPressed(VK_OEM_MINUS) || win0Input.isTriggered(VK_OEM_MINUS))
        {
          if (s_gamma.x != sc_gammaMin)
          {
            s_gamma.x = glm::clamp(s_gamma.x - 0.125f, sc_gammaMin, sc_gammaMax);
            s_gamma.y = 1.0f / s_gamma.x;
            printf_s("Gamma decreased: %.3f\n", s_gamma.x);
          }
        }

        upVKWin->createAndSetPipeline(skullPipeline);
        { // skull object
          glm::mat4 xform{ cam.m_W2V * skullInfo.m_M2W };
          skullPipeline.pushConstant(FCB, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(xform), &xform);
          skullPipeline.pushConstant(FCB, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(s_gamma.y), &s_gamma.y);
          skullModel.draw(FCB);
        }

        upVKWin->createAndSetPipeline(carPipeline);
        { // car object
          glm::mat4 xform{ cam.m_W2V * carInfo.m_M2W };
          carPipeline.pushConstant(FCB, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(xform), &xform);
          carPipeline.pushConstant(FCB, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(s_gamma.y), &s_gamma.y);
          carModel.draw(FCB);
        }

        upVKWin->FrameEnd();
        upVKWin->PageFlip();
        // ******************************************** RENDER LOOP END ****
        // *****************************************************************
      }
      // ************************************************** WINDOW LOOP END ****
      // ***********************************************************************
    }
    carModel.destroyModel();
    skullModel.destroyModel();
    upVKWin->destroyPipelineInfo(carPipeline);
    upVKWin->destroyPipelineInfo(skullPipeline);
  }

  FinalCar::unloadTextures(CarTextures);
  FinalSkull::unloadTextures(SkullTextures);

  return 0;

}
