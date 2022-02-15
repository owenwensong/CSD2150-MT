/*!*****************************************************************************
 * @file    graphicsHandler_vulkan.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    12 FEB 2022
 * @brief   This is the interface for the graphicsHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef GRAPHICS_HANDLER_VULKAN_HEADER
#define GRAPHICS_HANDLER_VULKAN_HEADER

#include <memory>   // std::shared_ptr
#include <utility/Singleton.h>
#include <vulkanHelpers/vulkanInstance.h>
#include <vulkanHelpers/vulkanDevice.h>
#include <vulkanHelpers/vulkanWindow.h>
#include <vector>

class graphicsHandler : public Singleton<graphicsHandler>
{
public:

    static constexpr size_t flagDebugPrint      { 0b0001 };
    static constexpr size_t flagDebugLayer      { 0b0010 };
    static constexpr size_t flagRenderDocLayer  { 0b0100 };

    bool OK() const noexcept;

    ~graphicsHandler();

    bool processInputEvents();// rename everything? TMP TMP TMP TMP TMP

private:
    friend class Singleton;
    graphicsHandler& operator=(graphicsHandler const&) = delete;
    graphicsHandler(graphicsHandler const&) = delete;
private:

    graphicsHandler(size_t flagOptions);

    std::shared_ptr<vulkanInstance> m_pVKInst;  // shared so stuff can depend on it
    std::shared_ptr<vulkanDevice> m_pVKDevice;  // has a copy of m_pVKInst
    std::unique_ptr<vulkanWindow> m_pVKWindow;  // has a copy of m_pVKDevice

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield bDebugPrint : 1;   // does not affect error/warning printouts

};

#endif//GRAPHICS_HANDLER_VULKAN_HEADER
