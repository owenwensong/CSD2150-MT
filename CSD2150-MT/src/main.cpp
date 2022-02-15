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

void destroyHandlers()
{
    //windowHandler::destroyInstance();
    graphicsHandler::destroyInstance();
}

int main()
{
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//DEBUG || 
    //char* leak{ new char[69] };

    std::atexit(destroyHandlers);// destroy handlers for any exit

    graphicsHandler* pGH
    {
        graphicsHandler::createInstance
        (
            graphicsHandler::flagDebugPrint | 
            graphicsHandler::flagDebugLayer | 
            graphicsHandler::flagRenderDocLayer
        )
    };
    if (pGH == nullptr || !pGH->OK())
    {
        printf_s("FAILED TO CREATE GRAPHICS HANDLER\n");
        return -3;
    }

    while (pGH->processInputEvents())
    {
        // do nothing lmao TMP TMP TMP
    }

    return 0;

}
