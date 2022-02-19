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
        printf_s("FAILED TO CREATE WINDOW HANDLER\n");
        return -3;
    }

    if (std::unique_ptr<vulkanWindow> upVKWin{ pWH->createWindow(windowSetup{.m_Title{L"TestWindow"sv}}) }; upVKWin && upVKWin->OK())
    {
        windowsInput& win0Input{ upVKWin->m_windowsWindow.m_windowInputs };
        printf_s("AAAA\n");
        VkShaderModule fragShader{ pWH->createShaderModule("../Assets/Shaders/triangleFrag.spv") };
        printf_s("BBBB\n");
        VkShaderModule vertShader{ pWH->createShaderModule("../Assets/Shaders/triangleVert.spv") };
        printf_s("CCCC\n");
        VkPipelineLayout pipelineLayout
        {
            pWH->createPipelineLayout
            (
                VkPipelineLayoutCreateInfo
                {
                    .sType{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO },
                    .setLayoutCount         { 0 },
                    .pSetLayouts            { nullptr },
                    .pushConstantRangeCount { 0 },
                    .pPushConstantRanges    { nullptr }
                }
            )
        };

        vulkanPipeline trianglePipeline;
        pWH->createPipelineInfo(trianglePipeline, vertShader, fragShader);

        while (pWH->processInputEvents())
        {
            win0Input.update();
            win0Input.debugPrint(0b0101);

            // FCB stands for Frame Command Buffer, this frame's command buffer!
            if (VkCommandBuffer FCB{ upVKWin->FrameBegin() }; FCB != VK_NULL_HANDLE)
            {
                static uint32_t counter{ 0 };
                // stuff
                static float R{ 0.0f };
                static float G{ 0.0f };
                static float B{ 0.0f };
                switch (++counter)
                {
                case 3000:  R = 0.5f; G = 0.0f; B = 0.0f; break;
                case 6000:  R = 0.0f; G = 0.5f; B = 0.0f; break;
                case 9000:  R = 0.0f; G = 0.0f; B = 0.5f; break;
                case 12000: R = 0.5f; G = 0.5f; B = 0.5f; break;
                case 15000: R = 0.0f; G = 0.0f; B = 0.0f; counter = 0; break;
                default: break;
                }
                upVKWin->m_VKClearValue[0].color.float32[0] = R;
                upVKWin->m_VKClearValue[0].color.float32[1] = G;
                upVKWin->m_VKClearValue[0].color.float32[2] = B;

                upVKWin->createAndSetPipeline(trianglePipeline, pipelineLayout);
                vkCmdDraw(FCB, 3, 1, 0, 0);

                // end of stuff
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
