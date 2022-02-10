/*!*****************************************************************************
 * @file    WindowHandler_windows.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    09 FEB 2022
 * @brief   This is the interface for the WindowHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef WINDOW_HANDLER_WINDOWS_HEADER
#define WINDOW_HANDLER_WINDOWS_HEADER

#include <utility/Singleton.h>
#include <utility/windowsInclude.h>

class windowHandler : public Singleton<windowHandler>
{
public:
    
    bool createWindow(int width, int height, bool borderlessFullscreen = false);

    HWND getWindowHandle() const noexcept;

private:
    friend class Singleton;
    windowHandler& operator=(windowHandler const&) = delete;
    windowHandler(windowHandler const&) = delete;
    // using preprocessor definition with ## results in weird intellisense bug
private: 
    
    windowHandler();

    bool registerWindowClass() noexcept;

    HWND m_pWindowHandle;

    using bitfield = size_t;

};

#endif//WINDOW_HANDLER_WINDOWS_HEADER
