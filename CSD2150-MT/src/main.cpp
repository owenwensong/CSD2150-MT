/*!*****************************************************************************
 * @file    main.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    09 FEB 2022
 * @brief   This is the entry point of the program.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <memory>
#include <handlers/graphicsHandler_vulkan.h>
#include <handlers/WindowHandler_windows.h>
#include <handlers/inputHandler_windows.h>

void destroyHandlers()
{
    windowHandler::destroyInstance();
    graphicsHandler::destroyInstance();
}

int main()
{
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//DEBUG || 
    //char* leak{ new char[69] };

    graphicsHandler* pGH{ graphicsHandler::createInstance(true, true) };
    if (pGH == nullptr || !pGH->VkInstanceOK())
    {
        printf_s("FAILED TO CREATE GRAPHICS HANDLER\n");
        destroyHandlers();
        return -3;
    }

    windowHandler* pWH{ windowHandler::createInstance() };
    if (pWH == nullptr)
    {
        printf_s("FAILED TO CREATE WINDOW HANDLER\n");
        return -1;
    }

    if (pWH->createWindow(1280, 720, false) == false)
    {
        printf_s("FAILED TO CREATE WINDOW\n");
        destroyHandlers();
        return -2;
    }

    inputHandler::initialize();

    for (bool bLoop{ true }; bLoop;)
    {
        for (MSG msg; PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE); DispatchMessage(&msg))
        {
            if (msg.message == WM_QUIT)bLoop = false;
        }
        // loop here?
        inputHandler::update();
        inputHandler::debugPrint(0b0101);
        if (inputHandler::isTriggered(VK_ESCAPE))bLoop = false;
    }

    destroyHandlers();

    return 0;

}
