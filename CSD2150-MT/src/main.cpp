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
        }
    }

    return 0;

}
