/*!*****************************************************************************
 * @file    vulkanWindow.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 FEB 2022
 * @brief   This is the implementation of the vulkan window class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/vulkanWindow.h>

bool vulkanWindow::Initialize(std::shared_ptr<vulkanDevice>& Device, 
                              windowSetup const& Setup)
{
    m_Device = Device;
    m_bfClearOnRender = Setup.m_bClearOnRender ? 1 : 0;
    m_VKClearValue[0] = VkClearValue
    { .color{ .float32{  
        Setup.m_ClearColorR,
        Setup.m_ClearColorG,
        Setup.m_ClearColorB,
        Setup.m_ClearColorA
    } } };

    std::shared_ptr<vulkanInstance>& VkInst{ m_Device->getVKInst() };

    if (false == m_windowsWindow.createWindow(Setup))
    {
        printWarning("FAILED TO CREATE WINDOWS WINDOW"sv, true);
        return false;
    }
    // same as OK, but I need the HWND to set data
    if (HWND winHandle{ m_windowsWindow.getSystemWindowHandle() }; winHandle != nullptr)
    {
        SetWindowLongPtr(winHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }
    else
    {
        printWarning("HWND WAS SOMEHOW INVALID DESPITE OK WINDOW"sv, true);
        return false;
    }

    // Create surface and stuff, returning now just to test
    return true;

}
