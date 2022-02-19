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

    if (std::unique_ptr<vulkanWindow> upVKWin{ pWH->createWindow(windowSetup{ .m_ClearColorR{ 0.0f }, .m_ClearColorG{ 0.0f }, .m_ClearColorB{ 0.0f }, .m_Title{ L"TestWindow"sv } }) }; upVKWin && upVKWin->OK())
    {
        windowsInput& win0Input{ upVKWin->m_windowsWindow.m_windowInputs };
        VkShaderModule fragShader{ pWH->createShaderModule("../Assets/Shaders/triangleFrag.spv") };
        VkShaderModule vertShader{ pWH->createShaderModule("../Assets/Shaders/triangleVert.spv") };
        VkPushConstantRange pushConstantRange
        {
            .stageFlags{ VK_SHADER_STAGE_VERTEX_BIT },
            .offset{ 0 },
            .size{ sizeof(float) },
        };
        VkPipelineLayout pipelineLayout
        {
            pWH->createPipelineLayout
            (
                VkPipelineLayoutCreateInfo
                {
                    .sType{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO },
                    .setLayoutCount         { 0 },
                    .pSetLayouts            { nullptr },
                    .pushConstantRangeCount { 1 },                  // just 1
                    .pPushConstantRanges    { &pushConstantRange }  // to push winsize
                }
            )
        };

        vulkanPipeline trianglePipeline;
        pWH->createPipelineInfo(trianglePipeline, vertShader, fragShader);

        // only enter loop if everything OK
        if (fragShader == VK_NULL_HANDLE || 
            vertShader == VK_NULL_HANDLE || 
            pipelineLayout == VK_NULL_HANDLE)
        {
            printWarning("pipeline prep failed"sv, true);
            
        }
        else while (pWH->processInputEvents())
        {
            win0Input.update();
            win0Input.debugPrint(0b0101);

            if (win0Input.isTriggered(VK_F11))upVKWin->toggleFullscreen();

            // FCB stands for Frame Command Buffer, this frame's command buffer!
            if (VkCommandBuffer FCB{ upVKWin->FrameBegin() }; FCB != VK_NULL_HANDLE)
            {
                upVKWin->createAndSetPipeline(trianglePipeline, pipelineLayout);

                float aspectRatioYX{ static_cast<float>(upVKWin->m_windowsWindow.getHeight()) / upVKWin->m_windowsWindow.getWidth() };
                vkCmdPushConstants(FCB, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, static_cast<uint32_t>(sizeof(float)), &aspectRatioYX);
                vkCmdDraw(FCB, 3, 1, 0, 0);

                upVKWin->FrameEnd();
                upVKWin->PageFlip();
            }
        }

        pWH->destroyPipelineLayout(pipelineLayout);
        pWH->destroyShaderModule(vertShader);
        pWH->destroyShaderModule(fragShader);

    }

    return 0;

}
