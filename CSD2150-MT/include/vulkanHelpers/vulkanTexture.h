/*!*****************************************************************************
 * @file    vulkanTexture.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    06 APR 2022
 * @brief   This is the interface for the vulkanTexture struct
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_TEXTURE_HELPER_HEADER
#define VULKAN_TEXTURE_HELPER_HEADER

#include <vulkan/vulkan.h>
#include <filesystem>

struct vulkanTexture
{
  static constexpr VkFlags s_ImageUsage_Sampler { VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
  static constexpr VkFlags s_MemPropFlag_Sampler{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };



  struct Setup
  {
    std::filesystem::path m_Path;
    VkSamplerAddressMode  m_AddressModeU{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
    VkSamplerAddressMode  m_AddressModeV{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
    VkSamplerAddressMode  m_AddressModeW{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
    VkImageUsageFlags     m_Usage   { s_ImageUsage_Sampler };
    VkImageTiling         m_Tiling  { VK_IMAGE_TILING_OPTIMAL };
    VkSampleCountFlagBits m_Samples { VK_SAMPLE_COUNT_1_BIT };
  };

  VkExtent3D      m_Extent  { .width{ 0 }, .height{ 0 }, .depth{ 0 } };
  VkImage         m_Image   { VK_NULL_HANDLE };
  VkDeviceMemory  m_Memory  { VK_NULL_HANDLE };
  VkImageView     m_View    { VK_NULL_HANDLE };
  VkSampler       m_Sampler { VK_NULL_HANDLE };
};

#endif//VULKAN_TEXTURE_HELPER_HEADER
