/*!*****************************************************************************
 * @file    windowsWindow.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 FEB 2022
 * @brief   This is the interface of the windows window class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef WINDOWS_WINDOW_HELPER_HEADER
#define WINDOWS_WINDOW_HELPER_HEADER

#include <windowsHelpers/windowsInput.h>
#include <vulkanHelpers/printWarnings.h>
#include <utility/windowsInclude.h>
#include <tuple>

struct windowSetup;  // put here to prevent include loops


class windowsWindow
{
public:
    
    static constexpr std::wstring_view defaultWindowTitle{ L"OVKWindow"sv };

    windowsWindow();    // no RAII to follow the rest of the stuff going on
    ~windowsWindow();

    int getWidth() const noexcept;
    int getHeight() const noexcept;

    bool OK() const noexcept;
    bool isFullscreen() const noexcept;
    bool isMinimized() const noexcept;
    bool isResized() const noexcept;
    bool isFocused() const noexcept;

    void resetResized() noexcept;

    HWND getSystemWindowHandle() const noexcept;

    std::tuple<int, int> getPosition() const;

    bool createWindow(windowSetup const& Setup);

    void setFullscreen(bool fullscreenMode) noexcept;

    void setWindowedWidth(int width) noexcept;

    void setWindowedHeight(int height) noexcept;

private:

    static constexpr int minWindowSizeX{ 800 };
    static constexpr int minWindowSizeY{ 600 };

    static bool registerWindowClass() noexcept;

    RECT getAdjustedRect() noexcept;
    
public:

    windowsInput    m_windowInputs  {  };

private:
    HWND            m_hWindow       { nullptr };

public: // why is this needed so much by xgpu

    int             m_windowedWidth { minWindowSizeX };
    int             m_windowedHeight{ minWindowSizeY };
    int             m_Width         { 0 };
    int             m_Height        { 0 };

public: // set public to adjust from inside WHWndProc
    
    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield        m_bfFullscreen  { 0 };
    bitfield        m_bfMinimized   { 0 };
    bitfield        m_bfResized     { 0 };

};

struct windowSetup  // put here to prevent include loops
{
    int m_Width{ 1280 };
    int m_Height{ 720 };
    bool m_bFullscreen{ false };
    bool m_bClearOnRender{ true };
    bool m_bSyncOn{ false };
    float m_ClearColorR{ 0.45f };
    float m_ClearColorG{ 0.45f };
    float m_ClearColorB{ 0.45f };
    float m_ClearColorA{ 1.0f };
    std::wstring_view m_Title{ windowsWindow::defaultWindowTitle };
};

#endif//WINDOWS_WINDOW_HELPER_HEADER
