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

    for (MSG msg; GetMessage(&msg, nullptr, 0, 0);)
    {
        //TranslateMessage(&msg);
        DispatchMessage(&msg);

        //static int i{ 0 };
        //printf_s("i: %d\n", i++);
    }

    windowHandler::destroyInstance();

    return 0;

}
