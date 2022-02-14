/*!*****************************************************************************
 * @file    vulkanDevice.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    13 FEB 2022
 * @brief   This is the implementation for the vulkan device class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <handlers/graphicsHandler_vulkan.h>    // no circular, not in header. (still bad though)
#include <vulkanHelpers/vulkanDevice.h>
#include <vulkanHelpers/printWarnings.h>
#include <algorithm>
#include <array>

std::vector<VkPhysicalDevice> collectPhysicalDevices(VkInstance pVKInstT)
{
    
    // get the number of physical devices
    uint32_t numPhysicalDevices{ 0 };
    if (VkResult tmpRes{ vkEnumeratePhysicalDevices(pVKInstT, &numPhysicalDevices, nullptr) }; tmpRes != VK_SUCCESS || numPhysicalDevices <= 0)
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

    if (VkResult tmpRes{ vkEnumeratePhysicalDevices(pVKInstT, &numPhysicalDevices, retval.data()) }; tmpRes != VK_SUCCESS)
    {
        printVKWarning(tmpRes, "Vulkan could not enumerate physical devices"sv, true);
        return { /* return empty vector */ };
    }

    // Filter out devices that don't support the graphicsHandler apiVersion
    for (size_t i{ 0 }, t{ retval.size() }; i < t; ++i)
    {
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(retval[i], &DeviceProperties);
        if (DeviceProperties.apiVersion < graphicsHandler::apiVersion)
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

bool vulkanDevice::createGraphicsDevice(std::vector<VkQueueFamilyProperties> const& DeviceProperties, bool validationLayersOn)
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
    if (validationLayersOn)
    {
        ValidationLayers = getValidationLayers();
        deviceCreateInfo.enabledLayerCount = static_cast<decltype(VkDeviceCreateInfo::enabledLayerCount)>(ValidationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }

    // nullptr allocator hardcoded here, "this is not meant as a performance feature" anyway
    if (VkResult tmpRes{ vkCreateDevice(m_VKPhysicalDevice, &deviceCreateInfo, nullptr, &m_VKDevice) }; tmpRes != VK_SUCCESS)
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

vulkanDevice::vulkanDevice(VkInstance pVKInstT, bool validationLayersOn) : 
    vulkanDevice{}
{
    createThisDevice(pVKInstT, validationLayersOn);
}

bool vulkanDevice::createThisDevice(VkInstance pVKInstT, bool validationLayersOn)
{
    if (isCreated)return true;
    std::vector<VkPhysicalDevice> PhysicalDevices{ collectPhysicalDevices(pVKInstT) };
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
                if (initialize(QueueIndex, PhysicalDevice, std::move(DeviceProps), validationLayersOn))
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

bool vulkanDevice::initialize(uint32_t MainQueueIndex, VkPhysicalDevice PhysicalDevice, std::vector<VkQueueFamilyProperties> Properties, bool validationLayersOn)
{
    m_VKPhysicalDevice  = PhysicalDevice;
    m_MainQueueIndex    = MainQueueIndex;

    if (createGraphicsDevice(Properties, validationLayersOn) == false)
    {
        return false;// error already printed inside
    }

    // Get all the queues (NO MUTEX INTEGRATION, WILL I NEED IT??????)
    vkGetDeviceQueue(m_VKDevice, m_MainQueueIndex, 0, &m_VKMainQueue);
    vkGetDeviceQueue(m_VKDevice, m_TransferQueueIndex, 0, &m_VKTransferQueue);

    // Gather physical device memory properties
    vkGetPhysicalDeviceMemoryProperties(m_VKPhysicalDevice, &m_VKDeviceMemoryProperties);
    vkGetPhysicalDeviceProperties(m_VKPhysicalDevice, &m_VKPhysicalDeviceProperties);

    // Create the Pipeline Cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO
    };
    // nullptr allocator hardcoded here, "this is not meant as a performance feature" anyway
    if (VkResult tmpRes{ vkCreatePipelineCache(m_VKDevice, &pipelineCacheCreateInfo, nullptr, &m_VKPipelineCache) }; tmpRes != VK_SUCCESS)
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

        if (VkResult tmpRes{ vkCreateDescriptorPool(m_VKDevice, &PoolCreateInfo, nullptr, &m_VKDescriptorPool)}; tmpRes != VK_SUCCESS)
        {
            printVKWarning(tmpRes, "Failed to created a Descriptor Pool"sv, true);
            return false;
        }

    }

    return true;
}

