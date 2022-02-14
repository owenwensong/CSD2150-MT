/*!*****************************************************************************
 * @file    vulkanDevice.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    13 FEB 2022
 * @brief   This is the interface for the vulkan device class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_DEVICE_HELPER_HEADER
#define VULKAN_DEVICE_HELPER_HEADER

#include <vulkan/vulkan.h>
#include <vector>

class vulkanDevice
{
public:

    bool createThisDevice(VkInstance pVKInstT, bool validationLayersOn);

    // ????????????
    bool initialize(uint32_t MainQueueIndex, VkPhysicalDevice PhysicalDevice, std::vector<VkQueueFamilyProperties> Properties, bool validationLayersOn);
    bool createGraphicsDevice(std::vector<VkQueueFamilyProperties> const& DeviceProperties, bool validationLayersOn);

    bool OK() const noexcept;
    vulkanDevice();
    vulkanDevice(VkInstance pVKInstT, bool validationLayersOn);

private:
    
    VkPhysicalDevice m_VKPhysicalDevice{};
    VkDevice m_VKDevice{};
    uint32_t m_MainQueueIndex{};
    uint32_t m_TransferQueueIndex{};
    VkQueue m_VKMainQueue{};// try without lock ???
    VkQueue m_VKTransferQueue{};// only 1 thread can submit queue at a time?
    VkPhysicalDeviceMemoryProperties m_VKDeviceMemoryProperties{};
    VkPipelineCache m_VKPipelineCache{};// if this is ?? for you what is it to me
    VkDeviceSize m_BufferMemoryAlignment{ 256 };
    VkPhysicalDeviceProperties m_VKPhysicalDeviceProperties{};
    VkDescriptorPool m_VKDescriptorPool{};// try without lock

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield isCreated : 1; // has this already been created?

};

#endif//VULKAN_DEVICE_HELPER_HEADER

