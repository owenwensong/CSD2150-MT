/*!*****************************************************************************
 * @file    WindowHandler_windows.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    09 FEB 2022
 * @brief   This is the implementation for the WindowHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <handlers/WindowHandler_windows.h>
#include <handlers/inputHandler_windows.h>  // to let it use WHWndProc
#include <cstdio>

#define FULL_STYLE_HELPER (WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define WINDOWED_STYLE_HELPER (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define FULL_EX_STYLE_HELPER (WS_EX_APPWINDOW)
#define WINDOWED_EX_STYLE_HELPER (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE)

// ************************************************************ TOP WNDPROC ****

LRESULT CALLBACK WHWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CHAR:// needs translatemessage for this
        printf_s("pressed: %c\n", static_cast<char>(wParam));
        break;
    case WM_LBUTTONDOWN:
        inputHandler::setVKTrigger(VK_LBUTTON);
        break;
    case WM_LBUTTONUP:
        inputHandler::setVKRelease(VK_LBUTTON);
        break;
    case WM_RBUTTONDOWN:
        inputHandler::setVKTrigger(VK_RBUTTON);
        break;
    case WM_RBUTTONUP:
        inputHandler::setVKRelease(VK_RBUTTON);
        break;
    case WM_MBUTTONDOWN:
        inputHandler::setVKTrigger(VK_MBUTTON);
        break;
    case WM_MBUTTONUP:
        inputHandler::setVKRelease(VK_MBUTTON);
        break;
    case WM_XBUTTONDOWN: // use higher order value masks
        if (wParam & 0x10000)inputHandler::setVKTrigger(VK_XBUTTON1);
        if (wParam & 0x20000)inputHandler::setVKTrigger(VK_XBUTTON2);
        break;
    case WM_XBUTTONUP:   // use higher order value masks
        if (wParam & 0x10000)inputHandler::setVKRelease(VK_XBUTTON1);
        if (wParam & 0x20000)inputHandler::setVKRelease(VK_XBUTTON2);
        break;
    case WM_KEYDOWN:
        if (lParam & 0x40000000)break;// ignore repeat message
        inputHandler::setVKTrigger(static_cast<inputHandler::keyIdx_T>(wParam));
        break;
    case WM_KEYUP:
        inputHandler::setVKRelease(static_cast<inputHandler::keyIdx_T>(wParam));
        break;
    case WM_SYSCOMMAND:
        if (SC_KEYMENU == wParam)return 0;// disable alt-space
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// *****************************************************************************

windowHandler::windowHandler() : 
    m_pWindowHandle{ nullptr }
{
    printf_s("window class registration: %s\n", registerWindowClass() ? "successful" : "failed");
}

bool windowHandler::registerWindowClass() noexcept
{
    HINSTANCE hInst{ GetModuleHandle(NULL) };
    if (WNDCLASS checkClass{}; GetClassInfo(hInst, L"OEWinClass", &checkClass))
    {
        return true;// class has been registered before
    }
    WNDCLASSEX windowClass
    {
        .cbSize = sizeof(decltype(windowClass)),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WHWndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInst,
        .hIcon = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)), // vs (HBRUSH)(COLOR_WINDOW+1); ?
        .lpszMenuName = NULL,
        .lpszClassName = L"OEWinClass",
        .hIconSm = LoadIcon(NULL, IDI_QUESTION)
    };

    return RegisterClassEx(&windowClass) ? true : false;
    
}

bool windowHandler::createWindow(int width, int height, bool borderlessFullscreen)
{
    if (m_pWindowHandle != nullptr)return true; // window already exists
    HINSTANCE hInst{ GetModuleHandle(NULL) };

    const int screenWidth{ GetSystemMetrics(SM_CXSCREEN) };
    const int screenHeight{ GetSystemMetrics(SM_CYSCREEN) };

    const DWORD windowStyle{ static_cast<DWORD>(borderlessFullscreen ? FULL_STYLE_HELPER : WINDOWED_STYLE_HELPER) };
    const DWORD windowExStyle{ static_cast<DWORD>(borderlessFullscreen ? FULL_EX_STYLE_HELPER : WINDOWED_EX_STYLE_HELPER) };
    // I assume EX_APPWINDOW_EDGE helps with window's fixed invisible borders

    RECT windowRect;
    if (borderlessFullscreen)
    {
        windowRect.left   = static_cast<decltype(windowRect.left)>(0);
        windowRect.right  = static_cast<decltype(windowRect.left)>(screenWidth);
        windowRect.top    = static_cast<decltype(windowRect.left)>(0);
        windowRect.bottom = static_cast<decltype(windowRect.left)>(screenHeight);
    }
    else
    {   // not messing with the math since integral division might make it off by 1px
        windowRect.left = static_cast<decltype(windowRect.left)>(screenWidth / 2 - width / 2);
        windowRect.right = static_cast<decltype(windowRect.left)>(width);
        windowRect.top = static_cast<decltype(windowRect.left)>(screenHeight / 2 - height / 2);
        windowRect.bottom = static_cast<decltype(windowRect.left)>(height);
    }

    AdjustWindowRectEx(&windowRect, windowStyle, FALSE, windowExStyle);

    m_pWindowHandle = CreateWindow
    (
        L"OEWinClass",      // class name
        L"WindowName",      // Window name
        windowStyle,        // window style
        windowRect.left,    // x
        windowRect.top,     // y
        windowRect.right,   // width (main monitor only rn)
        windowRect.bottom,  // height
        nullptr,            // hWndParent
        nullptr,            // menu
        hInst,              // hInstance
        nullptr             // lpParam
    );

    if (m_pWindowHandle == nullptr)return false;

    ShowWindow(m_pWindowHandle, SW_SHOW);
    UpdateWindow(m_pWindowHandle);
    //Menu = GetMenu(hWnd);// from my old window handler

    return true;
}

HWND windowHandler::getWindowHandle() const noexcept
{
    return m_pWindowHandle;
}
