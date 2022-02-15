/*!*****************************************************************************
 * @file    graphicsHandler_vulkan.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    12 FEB 2022
 * @brief   This is the implementation for the graphicsHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <handlers/graphicsHandler_vulkan.h>
#include <vulkanHelpers/printWarnings.h>
#include <iostream>


// ************************************************* GRAPHICS HANDLER CLASS ****

graphicsHandler::graphicsHandler(size_t flagOptions) :
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

graphicsHandler::~graphicsHandler()
{
		if (bDebugPrint)
		{
				std::cout << "graphicsHandler instance destruct!"sv << std::endl;
		}
}

bool graphicsHandler::OK() const noexcept
{
		return
		{
				(m_pVKInst.get()		!= nullptr && m_pVKInst->OK()		) &&
				(m_pVKDevice.get()	!= nullptr && m_pVKDevice->OK()	)
		};
}


// *****************************************************************************
