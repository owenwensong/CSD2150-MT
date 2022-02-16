/*!*****************************************************************************
 * @file    graphicsHandler_vulkan.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    12 FEB 2022
 * @brief   This is the implementation for the graphicsHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/printWarnings.h>
#include <handlers/windowHandler.h>
#include <iostream>


// *************************************************** WINDOW HANDLER CLASS ****

windowHandler::windowHandler(size_t flagOptions) :
		m_pVKInst
		{
				std::make_shared<vulkanInstance>
				(
						flagOptions & flagDebugLayer ? true : false, 
						flagOptions & flagRenderDocLayer ? true : false
				)
		},
		m_pVKDevice{ std::make_shared<vulkanDevice>(m_pVKInst) },
		bDebugPrint{ flagOptions & flagDebugPrint ? true : false }
{
		if (bDebugPrint)
		{
				std::cout << "graphicsHandler instance created! \nvulkanInstance status: "sv
									<< (m_pVKInst && m_pVKInst->OK() ? "OK"sv : "BAD"sv)
									<< "\nvulkanDevice status: "sv
									<< (m_pVKDevice && m_pVKDevice->OK() ? "OK"sv : "BAD"sv)
									<< std::endl;
		}

}

windowHandler::~windowHandler()
{
		if (bDebugPrint)
		{
				std::cout << "graphicsHandler instance destruct!"sv << std::endl;
		}
}

bool windowHandler::OK() const noexcept
{
		return
		{
				(m_pVKInst.get()		!= nullptr && m_pVKInst->OK()		) &&
				(m_pVKDevice.get()	!= nullptr && m_pVKDevice->OK()	)
		};
}

bool windowHandler::processInputEvents()
{
		for (MSG msg; PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE); DispatchMessage(&msg))
		{
				if (msg.message == WM_QUIT)return false;
		}
		return true;
}

std::unique_ptr<vulkanWindow> windowHandler::createWindow(windowSetup const& Setup)
{
		return std::make_unique<vulkanWindow>(m_pVKDevice, Setup);
}

// *****************************************************************************
