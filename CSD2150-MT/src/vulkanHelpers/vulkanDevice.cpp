/*!*****************************************************************************
 * @file    vulkanDevice.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    13 FEB 2022
 * @brief   This is the implementation for the vulkan device class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/printWarnings.h>
#include <vulkanHelpers/vulkanDevice.h>
#include <algorithm>
#include <array>

std::vector<VkPhysicalDevice> collectPhysicalDevices(vulkanInstance& vkInst)
{
    
    // get the number of physical devices
    uint32_t numPhysicalDevices{ 0 };
    if (VkResult tmpRes{ vkEnumeratePhysicalDevices(vkInst.m_VkHandle, &numPhysicalDevices, nullptr) }; tmpRes != VK_SUCCESS || numPhysicalDevices <= 0)
    {
        printVKWarning(tmpRes, "Unable to get Physical Device count"sv, true);
        return { /* return empty vector */ };
    }

    // initialize with just enough size
    std::vector<VkPhysicalDevice> retval
    {
        static_cast<decltype(retval)::size_type>(numPhysicalDevices),
        VK_NULL_HANDLE
    };

    if (VkResult tmpRes{ vkEnumeratePhysicalDevices(vkInst.m_VkHandle, &numPhysicalDevices, retval.data()) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Vulkan could not enumerate physical devices"sv, true);
        return { /* return empty vector */ };
    }

    // Filter out devices that don't support the graphicsHandler apiVersion
    for (size_t i{ 0 }, t{ retval.size() }; i < t; ++i)
    {
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(retval[i], &DeviceProperties);
        if (DeviceProperties.apiVersion < vulkanInstance::apiVersion)
        {
            retval.erase(retval.begin() + i);// remove by iter
            --i;// don't increment i on next loop since element removed
        }
    }
    
    // don't care about discrete/integrated for now

    if (retval.empty())printWarning("No physical devices present that support the required API version"sv, true);
    
    // push all discrete up to the front!
    auto cmp
    {   // a lot of unnecessary calls to vkGetPhysicalDeviceProperties but this 
        // only needs to happen once at load time, and also I expect very few 
        // devices. Crypto miners shouldn't be running this program anyway.
        +[](VkPhysicalDevice const& LHS, VkPhysicalDevice const& /*RHS*/)->bool
        {
            VkPhysicalDeviceProperties LHSP;
            //VkPhysicalDeviceProperties RHSP;
            vkGetPhysicalDeviceProperties(LHS, &LHSP);
            //vkGetPhysicalDeviceProperties(RHS, &RHSP);
            return LHSP.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            // I hope this doesn't result in some infinite loop
            // but since I will use stable_sort, things should remain where 
            // they are, meaning I can simply check LHS only
        }
    };

    // all discrete GPU will come before anything else
    std::stable_sort(retval.begin(), retval.begin() + retval.size() - 1, cmp);

    return retval;
}

bool vulkanDevice::createGraphicsDevice(std::vector<VkQueueFamilyProperties> const& DeviceProperties)
{
    for (uint32_t i{ 0 }, t{ static_cast<uint32_t>(DeviceProperties.size()) }; i < t; ++i)
    {
        if (DeviceProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && m_MainQueueIndex != i)
        {
            m_TransferQueueIndex = i;
            break;
        }
    }

    if (m_TransferQueueIndex == 0xFFFFFFFF)
    {
        printWarning("Unable to find a transfer only queue"sv);
        return false;
    }

    // Prepare queue info
    static const std::array queuePriorities{ 0.0f };// I assume static because this is actually used by the VKQueues and must exist somewhere?
    std::array queueCreateInfo
    {
        VkDeviceQueueCreateInfo
        {
            .sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex   = m_MainQueueIndex,
            .queueCount         = static_cast<decltype(VkDeviceQueueCreateInfo::queueCount)>(queuePriorities.size()),
            .pQueuePriorities   = queuePriorities.data()
        },
        VkDeviceQueueCreateInfo
        {
            .sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex   = m_TransferQueueIndex,
            .queueCount         = static_cast<decltype(VkDeviceQueueCreateInfo::queueCount)>(queuePriorities.size()),
            .pQueuePriorities   = queuePriorities.data()
        }
    };

    // Create device
    VkPhysicalDeviceFeatures Features;
    vkGetPhysicalDeviceFeatures(m_VKPhysicalDevice, &Features);
    Features.shaderClipDistance = true; // ??? overriding some stuff ???
    Features.shaderCullDistance = true;
    Features.samplerAnisotropy  = true;

    static constexpr std::array enabledExtensions
    {   //VK_NV_GLSL_SHADER_EXTENSION_NAME is deprecated, should not use.
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    VkDeviceCreateInfo deviceCreateInfo
    {
        .sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                      = nullptr,
        .queueCreateInfoCount       = static_cast<decltype(VkDeviceCreateInfo::queueCreateInfoCount)>(queueCreateInfo.size()),
        .pQueueCreateInfos          = queueCreateInfo.data(),
        .enabledLayerCount          = 0,
        .ppEnabledLayerNames        = nullptr,
        .enabledExtensionCount      = static_cast<decltype(VkDeviceCreateInfo::enabledExtensionCount)>(enabledExtensions.size()),
        .ppEnabledExtensionNames    = enabledExtensions.data(),
        .pEnabledFeatures           = &Features
    };

    std::vector<const char*> ValidationLayers;
    if (m_pVKInst->isDebugValidationOn())
    {
        ValidationLayers = getValidationLayers();
        deviceCreateInfo.enabledLayerCount = static_cast<decltype(VkDeviceCreateInfo::enabledLayerCount)>(ValidationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }

    if (VkResult tmpRes{ vkCreateDevice(m_VKPhysicalDevice, &deviceCreateInfo, m_pVKInst->m_pVKAllocator, &m_VKDevice) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to create the Vulkan Graphical Device"sv);
        return false;
    }

    return true;
}

bool vulkanDevice::OK() const noexcept
{
    return isCreated ? true : false;
}

vulkanDevice::vulkanDevice() : 
    isCreated{ 0 }
{

}

vulkanDevice::vulkanDevice(std::shared_ptr<vulkanInstance>& pVKInst) : 
    m_pVKInst{ pVKInst },
    isCreated{ 0 }
{
    if (m_pVKInst && m_pVKInst->OK())
    {
        createThisDevice();
    }
    else
    {
        printWarning("Unable to create vulkanDevice when vulkanInstance is invalid"sv, true);
    }
    
}

vulkanDevice::~vulkanDevice()
{
  vkDestroyCommandPool(m_VKDevice, m_TransferCommandSpecialPool, m_pVKInst->m_pVKAllocator);
  vkDestroyCommandPool(m_VKDevice, m_TransferCommandPool, m_pVKInst->m_pVKAllocator);
  std::scoped_lock Lk{ m_LockedVKDescriptorPool };// lock it and let it die
  vkDestroyDescriptorPool(m_VKDevice, m_LockedVKDescriptorPool.get(), m_pVKInst->m_pVKAllocator);
  vkDestroyPipelineCache(m_VKDevice, m_VKPipelineCache, m_pVKInst->m_pVKAllocator);
  vkDestroyDevice(m_VKDevice, m_pVKInst->m_pVKAllocator);
}

void vulkanDevice::waitForDeviceIdle()
{
  if (VkResult tmpRes{ vkDeviceWaitIdle(m_VKDevice) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "Failed to wait for device", true);
    // ???? Pretend there was no error ????
  }
}

bool vulkanDevice::createThisDevice(std::shared_ptr<vulkanInstance>* optionalOverride)
{
    if (isCreated)return true;
    if (optionalOverride != nullptr && *optionalOverride && (*optionalOverride)->OK())
    {
        m_pVKInst = *optionalOverride;
    }
    if (!m_pVKInst->OK())
    {
        printWarning("Attempting to create device with bad vulkanInstance"sv, true);
        return false;
    }

    std::vector<VkPhysicalDevice> PhysicalDevices{ collectPhysicalDevices(*m_pVKInst) };
    if (PhysicalDevices.empty())return false;// not OK! Where GPU

    // 
    for (VkPhysicalDevice const& PhysicalDevice : PhysicalDevices)
    {
        uint32_t PropertyCount{ 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &PropertyCount, nullptr);
        if (PropertyCount <= 0)
        {
            printWarning("Could not get device property count"sv);
            return false;
        }
        std::vector<VkQueueFamilyProperties> DeviceProps{ static_cast<decltype(DeviceProps)::size_type>(PropertyCount) };
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &PropertyCount, DeviceProps.data());
        if (PropertyCount <= 0)
        {   // if for some reason providing the pointer causes it to fail
            printWarning("Could not get device properties"sv);
            return false;
        }

        for (VkQueueFamilyProperties& Prop : DeviceProps)
        {
            if (Prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                // officially confused
                uint32_t QueueIndex{ static_cast<uint32_t>(&Prop - DeviceProps.data()) };
                if (initialize(QueueIndex, PhysicalDevice, std::move(DeviceProps)))
                {
                    isCreated = 1;
                    return true;
                }
                break;// must break since moving DeviceProps
            }
        }

    }

    printWarning("Failed to find a compatible device", true);
    return false;
}

bool vulkanDevice::initialize(uint32_t MainQueueIndex, VkPhysicalDevice PhysicalDevice, std::vector<VkQueueFamilyProperties> Properties)
{
    m_VKPhysicalDevice  = PhysicalDevice;
    m_MainQueueIndex    = MainQueueIndex;

    if (createGraphicsDevice(Properties) == false)
    {
        return false;// error already printed inside
    }

    // Get all the queues (NO MUTEX INTEGRATION, WILL I NEED IT??????)
    {
        std::scoped_lock Lks{ m_VKMainQueue, m_VKTransferQueue };
        vkGetDeviceQueue(m_VKDevice, m_MainQueueIndex, 0, &m_VKMainQueue.get());
        vkGetDeviceQueue(m_VKDevice, m_TransferQueueIndex, 0, &m_VKTransferQueue.get());
    }
    

    // Gather physical device memory properties
    vkGetPhysicalDeviceMemoryProperties(m_VKPhysicalDevice, &m_VKDeviceMemoryProperties);
    vkGetPhysicalDeviceProperties(m_VKPhysicalDevice, &m_VKPhysicalDeviceProperties);

    // Create the Pipeline Cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO
    };

    if (VkResult tmpRes{ vkCreatePipelineCache(m_VKDevice, &pipelineCacheCreateInfo, m_pVKInst->m_pVKAllocator, &m_VKPipelineCache) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Failed to Create the pipeline cache"sv, true);
        return false;
    }

    // create the DescriptorPool for pipelines
    // confusion 100
    {
        constexpr decltype(VkDescriptorPoolSize::descriptorCount) max_descriptors_per_pool_v{ 1000u };
        const std::array PoolSizes
        {
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,                   max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,             max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,             max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,      max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,      max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,            max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,    max_descriptors_per_pool_v },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,          max_descriptors_per_pool_v }
        };

        VkDescriptorPoolCreateInfo PoolCreateInfo
        {
            .sType          { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO },
            .flags          { VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT },
            .maxSets        { static_cast<decltype(VkDescriptorPoolCreateInfo::maxSets)>(max_descriptors_per_pool_v) },
            .poolSizeCount  { static_cast<decltype(VkDescriptorPoolCreateInfo::poolSizeCount)>(PoolSizes.size()) },
            .pPoolSizes     { PoolSizes.data() }
        };

        {   // from what I checked, scoped_lock is a superior lock_guard so I will use it instead even for single object locking
            std::scoped_lock Lk{ m_LockedVKDescriptorPool };
            if (VkResult tmpRes{ vkCreateDescriptorPool(m_VKDevice, &PoolCreateInfo, m_pVKInst->m_pVKAllocator, &m_LockedVKDescriptorPool.get())}; tmpRes != VK_SUCCESS)
            {
                printVKWarning(tmpRes, "Failed to created a Descriptor Pool"sv, true);
                return false;
            }
        }

    }

    {
      VkCommandPoolCreateInfo CreateInfo
      {
        .sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
        .flags{ VK_COMMAND_POOL_CREATE_TRANSIENT_BIT },
        .queueFamilyIndex{ m_TransferQueueIndex }
      };
      if (VkResult tmpRes{ vkCreateCommandPool(m_VKDevice, &CreateInfo, m_pVKInst->m_pVKAllocator, &m_TransferCommandPool) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create the Transfer command pool"sv, true);
        return false;
      }
    }
    {
      VkCommandPoolCreateInfo CreateInfo
      {
        .sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
        .flags{ VK_COMMAND_POOL_CREATE_TRANSIENT_BIT },
        .queueFamilyIndex{ m_MainQueueIndex }
      };
      if (VkResult tmpRes{ vkCreateCommandPool(m_VKDevice, &CreateInfo, m_pVKInst->m_pVKAllocator, &m_TransferCommandSpecialPool) }; tmpRes != VK_SUCCESS)
      {
        printVKWarning(tmpRes, "Unable to create the Transfer command pool"sv, true);
        return false;
      }
    }

    return true;
}

std::shared_ptr<vulkanInstance>& vulkanDevice::getVKInst()
{
    return m_pVKInst;
}

bool vulkanDevice::getMemoryType(uint32_t TypeBits, const VkFlags Properties, uint32_t& TypeIndex) const noexcept
{
    for (uint32_t i{ 0 }; i < 32; ++i, TypeBits >>= 1)
    {
        if (TypeBits & 1)
        {
            if (Properties == (m_VKDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties))
            {
                TypeIndex = i;
                return true;
            }
        }
    }
    printWarning("Failed to find memory flags"sv);
    return false;
}
