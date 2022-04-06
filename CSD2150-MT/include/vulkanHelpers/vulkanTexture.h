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

struct vulkanTexture
{
  VkImage         m_Image;
  VkDeviceMemory  m_Memory;
};

#endif//VULKAN_TEXTURE_HELPER_HEADER
