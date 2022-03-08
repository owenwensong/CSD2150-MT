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

    vulkanPipeline trianglePipeline;
    if (!pWH->createPipelineInfo
    (
      trianglePipeline,
      vulkanPipeline::setup
      {
        .m_PathShaderVert{ "../Assets/Shaders/triangleVert.spv"sv },
        .m_PathShaderFrag{ "../Assets/Shaders/triangleFrag.spv"sv },

        // uniform stuff here

        .m_PushConstantRangeVert{ vulkanPipeline::createPushConstantInfo<float>(VK_SHADER_STAGE_VERTEX_BIT) },
        .m_PushConstantRangeFrag{ vulkanPipeline::createPushConstantInfo<>(VK_SHADER_STAGE_FRAGMENT_BIT) },
      }
      )
      )
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

      // FCB stands for Frame Command Buffer, this frame's command buffer!
      if (VkCommandBuffer FCB{ upVKWin->FrameBegin() }; FCB != VK_NULL_HANDLE)
      {
      // *******************************************************************
      // ******************************************** RENDER LOOP BEGIN ****
        upVKWin->createAndSetPipeline(trianglePipeline);

        float aspectRatioYX{ static_cast<float>(upVKWin->m_windowsWindow.getHeight()) / upVKWin->m_windowsWindow.getWidth() };
        vkCmdPushConstants(FCB, trianglePipeline.m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(float)), &aspectRatioYX);
        vkCmdDraw(FCB, 3, 1, 0, 0);

        upVKWin->FrameEnd();
        upVKWin->PageFlip();
        // ******************************************** RENDER LOOP END ****
        // *****************************************************************
      }
      // ************************************************** WINDOW LOOP END ****
      // ***********************************************************************
    }
    pWH->destroyPipelineInfo(trianglePipeline);
  }

  return 0;

}
