/*!*****************************************************************************
 * @file    vulkanInstance.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    14 FEB 2022
 * @brief   This is the interface for the vulkan instance helper class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_HELPERS_VULKAN_INSTANCE_HEADER
#define VULKAN_HELPERS_VULKAN_INSTANCE_HEADER

#include <vulkan/vulkan.h>
#include <vector>

class vulkanInstance
{
public:

    static constexpr decltype(VkApplicationInfo::apiVersion) apiVersion{ VK_API_VERSION_1_2 };

    static VkInstance createVkInstance(bool enableDebugLayers, bool enableRenderDoc);

    bool OK() const noexcept;
    bool isDebugValidationOn() const noexcept;
    bool isRenderDocOn() const noexcept;

    vulkanInstance(bool enableDebugLayers, bool enableRenderDoc);
    ~vulkanInstance();

    // Data members
public:

    VkInstance m_VkHandle;
    VkAllocationCallbacks* m_pVKAllocator;

private:

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield bValidation : 1;   // add validation layers to the VkInstance
    bitfield bRenderDoc  : 1;   // add RenderDoc layer to the VkInstance
};

std::vector<const char*> getValidationLayers();// becoming spaghetti

#endif//VULKAN_HELPERS_VULKAN_INSTANCE_HEADER
