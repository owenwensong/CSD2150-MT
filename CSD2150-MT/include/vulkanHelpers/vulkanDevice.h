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

#include <vulkanHelpers/vulkanInstance.h>
#include <vulkan/vulkan.h>
#include <utility/lockableObject.hpp>
#include <memory>
#include <vector>

class vulkanDevice
{
public:

    bool createThisDevice(std::shared_ptr<vulkanInstance>* optionalOverride = nullptr);

    bool OK() const noexcept;
    vulkanDevice();
    vulkanDevice(std::shared_ptr<vulkanInstance>& pVKInst);
    ~vulkanDevice();

    void waitForDeviceIdle();

    bool getMemoryType(uint32_t TypeBits, const VkFlags Properties, uint32_t& TypeIndex) const noexcept;

    std::shared_ptr<vulkanInstance>& getVKInst();

private:
    // ????????????
    bool initialize(uint32_t MainQueueIndex, VkPhysicalDevice PhysicalDevice, std::vector<VkQueueFamilyProperties> Properties);
    bool createGraphicsDevice(std::vector<VkQueueFamilyProperties> const& DeviceProperties);
public: // just realease it... I don't know why everything must touch each other
    
    std::shared_ptr<vulkanInstance>     m_pVKInst;
    VkPhysicalDevice                    m_VKPhysicalDevice{};
    VkDevice                            m_VKDevice{};
    uint32_t                            m_MainQueueIndex{};
    uint32_t                            m_TransferQueueIndex{};
    lockableObject<VkQueue>             m_VKMainQueue{};
    lockableObject<VkQueue>             m_VKTransferQueue{};
    VkPhysicalDeviceMemoryProperties    m_VKDeviceMemoryProperties{};
    VkPipelineCache                     m_VKPipelineCache{};    // ??
    VkDeviceSize                        m_BufferMemoryAlignment{ 256 };
    VkPhysicalDeviceProperties          m_VKPhysicalDeviceProperties{};
    lockableObject<VkDescriptorPool>    m_LockedVKDescriptorPool{};
    VkCommandPool                       m_TransferCommandPool{};

    // for transfers that need dst to support graphics
    VkCommandPool                       m_TransferCommandSpecialPool{};

    using bitfield = intptr_t;  // bitfield size match ptr size

    bitfield isCreated : 1; // has this already been created?

};

#endif//VULKAN_DEVICE_HELPER_HEADER

