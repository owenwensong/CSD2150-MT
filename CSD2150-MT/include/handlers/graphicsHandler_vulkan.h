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

#include <utility/Singleton.h>
#include <vulkanHelpers/vulkanDevice.h>// contains vulkan.h
#include <vector>

class graphicsHandler : public Singleton<graphicsHandler>
{
public:

    static constexpr decltype(VkApplicationInfo::apiVersion) apiVersion{ VK_API_VERSION_1_2 };

    static VkInstance createVkInstance(bool enableDebugLayers, bool enableRenderDoc);

    bool VkInstanceOK() const noexcept;

    bool isDebugValidationOn() const noexcept;

    ~graphicsHandler();
private:
    friend class Singleton;
    graphicsHandler& operator=(graphicsHandler const&) = delete;
    graphicsHandler(graphicsHandler const&) = delete;
private:

    graphicsHandler(bool enableDebugLayers, bool enableRenderDoc);

    VkInstance m_VkHandle;
    vulkanDevice m_VKDevice;

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield bValidation : 1;

};

std::vector<const char*> getValidationLayers();// becoming spaghetti

#endif//GRAPHICS_HANDLER_VULKAN_HEADER
