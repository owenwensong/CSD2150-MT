/*!*****************************************************************************
 * @file    main.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    09 FEB 2022
 * @brief   This is the entry point of the program.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <memory>
#include <handlers/WindowHandler_windows.h>
#include <handlers/inputHandler_windows.h>

int main()
{
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//DEBUG || 
    //char* leak{ new char[69] };

    windowHandler* pWH{ windowHandler::createInstance() };
    if (pWH == nullptr)
    {
        printf_s("FAILED TO CREATE WINDOW HANDLER\n");
        return -1;
    }

    if (pWH->createWindow(1920, 1080, false) == false)
    {
        printf_s("FAILED TO CREATE WINDOW\n");
        return -2;
    }

    // no need accelerator table
    //HACCEL hAccelTable{ LoadAccelerators(GetModuleHandle(NULL), )}

    inputHandler::initialize();

    for (bool bLoop{ true }; bLoop;)
    {
        for (MSG msg; PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE); DispatchMessage(&msg))
        {
            if (msg.message == WM_QUIT)bLoop = false;
        }
        // loop here?
        inputHandler::update();
        inputHandler::debugPrint(0x101);
        if (inputHandler::isTriggered(VK_ESCAPE))bLoop = false;
    }
    //for (MSG msg; GetMessage(&msg, nullptr, 0, 0);)
    //{
    //    //TranslateMessage(&msg);
    //    DispatchMessage(&msg);
    //    inputHandler::update();

    //    inputHandler::debugPrint(0x111);

    //    //static int i{ 0 };
    //    //printf_s("i: %d\n", i++);
    //}

    windowHandler::destroyInstance();

    return 0;

}
