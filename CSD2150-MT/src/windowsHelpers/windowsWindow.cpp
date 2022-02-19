/*!*****************************************************************************
 * @file    windowsWindow.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 FEB 2022
 * @brief   This is the implementation of the windows window class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <windowsHelpers/windowsWindow.h>
#include <vulkanHelpers/vulkanWindow.h>     // hahaha

#define FULL_STYLE_HELPER (WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define WINDOWED_STYLE_HELPER (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define FULL_EX_STYLE_HELPER (WS_EX_APPWINDOW)
#define WINDOWED_EX_STYLE_HELPER (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE)

// *****************************************************************************
// ************************************************************** CTOR/DTOR ****

windowsWindow::windowsWindow()
{

}

windowsWindow::~windowsWindow()
{
    if (m_hWindow != nullptr)
    {
        DestroyWindow(m_hWindow);
    }
}

// *****************************************************************************
// **************************************************************** GETTERS ****

int windowsWindow::getWidth() const noexcept { return m_Width; }
int windowsWindow::getHeight() const noexcept { return m_Height; }

bool windowsWindow::OK() const noexcept
{
    return m_hWindow != nullptr;
}

bool windowsWindow::isFullscreen() const noexcept
{
    return m_bfFullscreen ? true : false;
}

bool windowsWindow::isMinimized() const noexcept
{
    return m_bfMinimized ? true : false;
}

bool windowsWindow::isResized() const noexcept
{
    return m_bfResized ? true : false;
}

bool windowsWindow::isFocused() const noexcept
{
    return m_hWindow == GetFocus();
}

void windowsWindow::resetResized() noexcept
{
    m_bfResized = 0;
}

HWND windowsWindow::getSystemWindowHandle() const noexcept
{
    return m_hWindow;
}

std::tuple<int, int> windowsWindow::getPosition() const
{
    POINT tmpRetval{ 0, 0 };
    ClientToScreen(m_hWindow, &tmpRetval);
    return { static_cast<int>(tmpRetval.x), static_cast<int>(tmpRetval.y) };
}

// *****************************************************************************
// ***************************************************** WINDOW CLASS STUFF ****

LRESULT CALLBACK WHWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    //GetWindowLongPtr(hWnd, GWLP_USERDATA)
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    //case WM_CHAR:// needs translatemessage for this
    //    printf_s("pressed: %c\n", static_cast<char>(wParam));
    //    break;
    case WM_LBUTTONDOWN:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKTrigger(VK_LBUTTON);
        }
        break;
    case WM_LBUTTONUP:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKRelease(VK_LBUTTON);
        }
        break;
    case WM_RBUTTONDOWN:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKTrigger(VK_RBUTTON);
        }
        break;
    case WM_RBUTTONUP:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKRelease(VK_RBUTTON);
        }
        break;
    case WM_MBUTTONDOWN:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKTrigger(VK_MBUTTON);
        }
        break;
    case WM_MBUTTONUP:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKRelease(VK_MBUTTON);
        }
        break;
    case WM_XBUTTONDOWN: // use higher order value masks
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            windowsInput& WI{ pVW->m_windowsWindow.m_windowInputs };
            if (wParam & 0x10000)WI.setVKTrigger(VK_XBUTTON1);
            if (wParam & 0x20000)WI.setVKTrigger(VK_XBUTTON2);
        }
        break;
    case WM_XBUTTONUP:   // use higher order value masks
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            windowsInput& WI{ pVW->m_windowsWindow.m_windowInputs };
            if (wParam & 0x10000)WI.setVKRelease(VK_XBUTTON1);
            if (wParam & 0x20000)WI.setVKRelease(VK_XBUTTON2);
        }
        break;
    case WM_KEYDOWN:
        if (lParam & 0x40000000)break;// ignore repeat message
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKTrigger(static_cast<windowsInput::keyIdx_T>(wParam));
        }
        break;
    case WM_KEYUP:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.setVKRelease(static_cast<windowsInput::keyIdx_T>(wParam));
        }
        break;
    case WM_MOUSEWHEEL: // No support for MOUSEHWHEEL right now
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.addMouseScroll(static_cast<short>(wParam >> 16 & 0xFFFF));
        }
        break;
    case WM_MOUSEMOVE:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            pVW->m_windowsWindow.m_windowInputs.updateCursorPos
            (
                static_cast<int>(lParam & 0xFFFF),
                static_cast<int>(lParam >> 16 & 0xFFFF)
            );
        }
        break;
    case WM_SYSCOMMAND:
        if (SC_KEYMENU == wParam)return 0;// disable alt-space
        break;
    case WM_SIZE:
        if (vulkanWindow* pVW{ reinterpret_cast<vulkanWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) }; pVW != nullptr)
        {
            switch (wParam)
            {
            case SIZE_MINIMIZED:
                pVW->m_windowsWindow.m_bfMinimized = 1;
                break;
            default:
                pVW->m_windowsWindow.m_Width  = static_cast<int>(lParam & 0xFFFF);
                pVW->m_windowsWindow.m_Height = static_cast<int>(lParam >> 16 & 0xFFFF);
                pVW->m_windowsWindow.m_bfResized = 1;
                pVW->m_windowsWindow.m_bfMinimized = 0;
                break;
            }
        }
        break;
    //case WM_SIZING:
    //    printf_s("RESIZING\n");
    //    break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool windowsWindow::registerWindowClass() noexcept
{
    HINSTANCE hInst{ GetModuleHandle(NULL) };
    if (WNDCLASS checkClass{}; GetClassInfo(hInst, L"OVKWinClass", &checkClass))
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
        .lpszClassName = L"OVKWinClass",
        .hIconSm = LoadIcon(NULL, IDI_QUESTION)
    };

    return RegisterClassEx(&windowClass) ? true : false;

}

bool windowsWindow::createWindow(windowSetup const& Setup)
{
    if (false == registerWindowClass())
    {
        printWarning("FAILED TO REGISTER WINDOW CLASS"sv, true);
        return false;
    }

    if (m_hWindow != nullptr)return true; // window already exists
    HINSTANCE hInst{ GetModuleHandle(NULL) };

    m_bfFullscreen = Setup.m_bFullscreen ? 1 : 0;

    m_windowedWidth = std::max(Setup.m_Width, minWindowSizeX);
    m_windowedHeight = std::max(Setup.m_Height, minWindowSizeY);

    const DWORD windowStyle{ m_bfFullscreen ? FULL_STYLE_HELPER : WINDOWED_STYLE_HELPER };
    RECT windowRect{ getAdjustedRect() };

    m_hWindow = CreateWindow
    (
        L"OVKWinClass",         // class name
        Setup.m_Title.data(),   // Window name
        windowStyle,            // window style
        windowRect.left,        // x
        windowRect.top,         // y
        windowRect.right,       // width (main monitor only rn)
        windowRect.bottom,      // height
        nullptr,                // hWndParent
        nullptr,                // menu
        hInst,                  // hInstance
        nullptr                 // lpParam
    );

    if (m_hWindow == nullptr)return false;

    ShowWindow(m_hWindow, SW_SHOW);
    UpdateWindow(m_hWindow);
    //Menu = GetMenu(hWnd);// from my old window handler

    return true;
}

RECT windowsWindow::getAdjustedRect() noexcept
{
    const int screenWidth{ GetSystemMetrics(SM_CXSCREEN) };
    const int screenHeight{ GetSystemMetrics(SM_CYSCREEN) };
    RECT retval;
    DWORD windowStyle, windowExStyle;
    if (m_bfFullscreen)
    {
        retval.left = static_cast<decltype(retval.left)>(0);
        retval.right = static_cast<decltype(retval.left)>(screenWidth);
        retval.top = static_cast<decltype(retval.left)>(0);
        retval.bottom = static_cast<decltype(retval.left)>(screenHeight);
        m_Width = screenWidth;
        m_Height = screenHeight;
        windowStyle = FULL_STYLE_HELPER;
        windowExStyle = FULL_EX_STYLE_HELPER;
    }
    else
    {   // not messing with the math since integral division might make it off by 1px
        retval.left = static_cast<decltype(retval.left)>(screenWidth / 2 - m_windowedWidth / 2);
        retval.right = static_cast<decltype(retval.left)>(m_windowedWidth);
        retval.top = static_cast<decltype(retval.left)>(screenHeight / 2 - m_windowedHeight / 2);
        retval.bottom = static_cast<decltype(retval.left)>(m_windowedHeight);
        m_Width = m_windowedWidth;    // maybe I should correct for the smaller 
        m_Height = m_windowedHeight;  // size from calling AdjustWindowRectEx?
        windowStyle = WINDOWED_STYLE_HELPER;
        windowExStyle = WINDOWED_EX_STYLE_HELPER;
    }
    // I assume EX_APPWINDOW_EDGE helps with window's fixed invisible borders
    AdjustWindowRectEx(&retval, windowStyle, FALSE, windowExStyle);

    return retval;
}

void windowsWindow::setFullscreen(bool fullscreenMode) noexcept
{
    if (m_bfFullscreen == (fullscreenMode ? 1 : 0))return;// already as desired

    // signed bitfields don't overflow to 0 :(
    m_bfFullscreen = fullscreenMode ? 1 : 0;

    RECT newWinRect{ getAdjustedRect() };
    
    // change window style to match
    SetWindowLongPtr(m_hWindow, GWL_STYLE, m_bfFullscreen ? FULL_STYLE_HELPER : WINDOWED_STYLE_HELPER);

    SetWindowPos
    (
        m_hWindow,
        nullptr,
        newWinRect.left,
        newWinRect.top,
        newWinRect.right,
        newWinRect.bottom,
        SWP_SHOWWINDOW
    );

    m_bfResized = true;
}

void windowsWindow::setWindowedWidth(int width) noexcept
{
    m_windowedWidth = width;
}

void windowsWindow::setWindowedHeight(int height) noexcept
{
    m_windowedHeight = height;
}

// *****************************************************************************
