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
        while (pWH->processInputEvents())
        {
            win0Input.update();
            win0Input.debugPrint(0b0101);
            if (upVKWin->FrameBegin())
            {
                static uint32_t counter{ 0 };
                // stuff
                static float R{ 0.0f };
                static float G{ 0.0f };
                static float B{ 0.0f };

                switch (++counter)
                {
                case 3000:
                    R = 1.0f; G = 0.0f; B = 0.0f;
                    break;
                case 6000:
                    R = 0.0f; G = 1.0f; B = 0.0f;
                    break;
                case 9000:
                    R = 0.0f; G = 0.0f; B = 1.0f;
                    break;
                case 12000:
                    R = 1.0f; G = 1.0f; B = 1.0f;
                    break;
                case 15000:
                    R = 0.0f; G = 0.0f; B = 0.0f;
                    counter = 0;
                    break;
                default:
                    break;
                }

                upVKWin->m_VKClearValue[0].color.float32[0] = R;
                upVKWin->m_VKClearValue[0].color.float32[1] = G;
                upVKWin->m_VKClearValue[0].color.float32[2] = B;
                // end of stuff
                upVKWin->FrameEnd();
                upVKWin->PageFlip();
            }
        }
    }

    return 0;

}
