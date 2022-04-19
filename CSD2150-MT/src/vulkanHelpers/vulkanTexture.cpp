/*!*****************************************************************************
 * @file    vulkanTexture.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    06 APR 2022
 * @brief   This is the implementation for the vulkanTexture struct
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/vulkanTexture.h>
#include <vulkanHelpers/printWarnings.h>
#include <handlers/windowHandler.h>
#pragma warning (disable : 4244 26451 26495 26812)// disable library warnings
#include <tinyddsloader.h>
#pragma warning (default : 4244 26451 26495)// reenable warnings except unscoped enum
#include <fstream>

bool tryTinyDDS(tinyddsloader::Result tDDSResult, bool isErrIfFail = false)
{
  switch (tDDSResult)
  {
  case tinyddsloader::Success: return true;
#define STRINGIFYDDSENUMHELPER(A, B) A##B
#define STRINGIFYDDSENUM(x) case tinyddsloader:: ##x : printWarning(STRINGIFYDDSENUMHELPER(#x, sv), isErrIfFail); return false
  STRINGIFYDDSENUM(ErrorFileOpen);
  STRINGIFYDDSENUM(ErrorRead);
  STRINGIFYDDSENUM(ErrorMagicWord);
  STRINGIFYDDSENUM(ErrorSize);
  STRINGIFYDDSENUM(ErrorVerify);
  STRINGIFYDDSENUM(ErrorNotSupported);
  STRINGIFYDDSENUM(ErrorInvalidData);
#undef STRINGIFYDDSENUMHELPER
#undef STRINGIFYDDSENUM
  default:
    printWarning("unknown tinyDDS error"sv, isErrIfFail);
    return false;
  }
}

static VkFormat DXGIFormattoVkFormat(tinyddsloader::DDSFile::DXGIFormat inFormat)
{
  using DDSDXGI = tinyddsloader::DDSFile::DXGIFormat;
  switch (inFormat)
  {
  case DDSDXGI::Unknown: return VkFormat::VK_FORMAT_UNDEFINED;
  //case tinyddsloader::DDSFile::DXGIFormat::R32G32B32A32_Typeless:
  case DDSDXGI::R32G32B32A32_Float:   return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
  case DDSDXGI::R32G32B32A32_UInt:    return VkFormat::VK_FORMAT_R32G32B32A32_UINT;
  case DDSDXGI::R32G32B32A32_SInt:    return VkFormat::VK_FORMAT_R32G32B32A32_SINT;
  //case DDSDXGI::R32G32B32_Typeless:
  case DDSDXGI::R32G32B32_Float:      return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
  case DDSDXGI::R32G32B32_UInt:       return VkFormat::VK_FORMAT_R32G32B32_UINT;
  case DDSDXGI::R32G32B32_SInt:       return VkFormat::VK_FORMAT_R32G32B32_SINT;
  //case DDSDXGI::R16G16B16A16_Typeless:
  case DDSDXGI::R16G16B16A16_Float:   return VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
  case DDSDXGI::R16G16B16A16_UNorm:   return VkFormat::VK_FORMAT_R16G16B16A16_UNORM;
  case DDSDXGI::R16G16B16A16_UInt:    return VkFormat::VK_FORMAT_R16G16B16A16_UINT;
  case DDSDXGI::R16G16B16A16_SNorm:   return VkFormat::VK_FORMAT_R16G16B16A16_SNORM;
  case DDSDXGI::R16G16B16A16_SInt:    return VkFormat::VK_FORMAT_R16G16B16A16_SINT;
  //case DDSDXGI::R32G32_Typeless:
  case DDSDXGI::R32G32_Float:         return VkFormat::VK_FORMAT_R32G32_SFLOAT;
  case DDSDXGI::R32G32_UInt:          return VkFormat::VK_FORMAT_R32G32_UINT;
  case DDSDXGI::R32G32_SInt:          return VkFormat::VK_FORMAT_R32G32_SINT;
  //case DDSDXGI::R32G8X24_Typeless:
  case DDSDXGI::D32_Float_S8X24_UInt: return VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;
  //case DDSDXGI::R32_Float_X8X24_Typeless:
  //case DDSDXGI::X32_Typeless_G8X24_UInt:
  //case DDSDXGI::R10G10B10A2_Typeless:
  //case DDSDXGI::R10G10B10A2_UNorm:
  //case DDSDXGI::R10G10B10A2_UInt:
  //case DDSDXGI::R11G11B10_Float:
  //case DDSDXGI::R8G8B8A8_Typeless:
  case DDSDXGI::R8G8B8A8_UNorm:       return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
  case DDSDXGI::R8G8B8A8_UNorm_SRGB:  return VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
  case DDSDXGI::R8G8B8A8_UInt:        return VkFormat::VK_FORMAT_R8G8B8A8_UINT;
  case DDSDXGI::R8G8B8A8_SNorm:       return VkFormat::VK_FORMAT_R8G8B8A8_SNORM;
  case DDSDXGI::R8G8B8A8_SInt:        return VkFormat::VK_FORMAT_R8G8B8A8_SINT;
  //case DDSDXGI::R16G16_Typeless:
  case DDSDXGI::R16G16_Float:         return VkFormat::VK_FORMAT_R16G16_SFLOAT;
  case DDSDXGI::R16G16_UNorm:         return VkFormat::VK_FORMAT_R16G16_UNORM;
  case DDSDXGI::R16G16_UInt:          return VkFormat::VK_FORMAT_R16G16_UINT;
  case DDSDXGI::R16G16_SNorm:         return VkFormat::VK_FORMAT_R16G16_SNORM;
  case DDSDXGI::R16G16_SInt:          return VkFormat::VK_FORMAT_R16G16_SINT;
  //case DDSDXGI::R32_Typeless:
  case DDSDXGI::D32_Float:            return VkFormat::VK_FORMAT_D32_SFLOAT;
  case DDSDXGI::R32_Float:            return VkFormat::VK_FORMAT_R32_SFLOAT;
  case DDSDXGI::R32_UInt:             return VkFormat::VK_FORMAT_R32_UINT;
  case DDSDXGI::R32_SInt:             return VkFormat::VK_FORMAT_R32_SINT;
  //case DDSDXGI::R24G8_Typeless:
  case DDSDXGI::D24_UNorm_S8_UInt:    return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;
  //case DDSDXGI::R24_UNorm_X8_Typeless:
  //case DDSDXGI::X24_Typeless_G8_UInt:
  //case DDSDXGI::R8G8_Typeless:
  case DDSDXGI::R8G8_UNorm:           return VkFormat::VK_FORMAT_R8G8_UNORM;
  case DDSDXGI::R8G8_UInt:            return VkFormat::VK_FORMAT_R8G8_UINT;
  case DDSDXGI::R8G8_SNorm:           return VkFormat::VK_FORMAT_R8G8_SNORM;
  case DDSDXGI::R8G8_SInt:            return VkFormat::VK_FORMAT_R8G8_SINT;
  //case DDSDXGI::R16_Typeless:
  case DDSDXGI::R16_Float:            return VkFormat::VK_FORMAT_R16_SFLOAT;
  case DDSDXGI::D16_UNorm:            return VkFormat::VK_FORMAT_D16_UNORM;
  case DDSDXGI::R16_UNorm:            return VkFormat::VK_FORMAT_R16_UNORM;
  case DDSDXGI::R16_UInt:             return VkFormat::VK_FORMAT_R16_UINT;
  case DDSDXGI::R16_SNorm:            return VkFormat::VK_FORMAT_R16_SNORM;
  case DDSDXGI::R16_SInt:             return VkFormat::VK_FORMAT_R16_SINT;
  //case DDSDXGI::R8_Typeless:
  case DDSDXGI::R8_UNorm:             return VkFormat::VK_FORMAT_R8_UNORM;
  case DDSDXGI::R8_UInt:              return VkFormat::VK_FORMAT_R8_UINT;
  case DDSDXGI::R8_SNorm:             return VkFormat::VK_FORMAT_R8_SNORM;
  case DDSDXGI::R8_SInt:              return VkFormat::VK_FORMAT_R8_SINT;
  case DDSDXGI::A8_UNorm:             return VkFormat::VK_FORMAT_R8_UNORM;
  //case DDSDXGI::R1_UNorm:
  //case DDSDXGI::R9G9B9E5_SHAREDEXP:
  //case DDSDXGI::R8G8_B8G8_UNorm:
  //case DDSDXGI::G8R8_G8B8_UNorm:
  //case DDSDXGI::BC1_Typeless:
  case DDSDXGI::BC1_UNorm:            return VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
  case DDSDXGI::BC1_UNorm_SRGB:       return VkFormat::VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
  //case DDSDXGI::BC2_Typeless:
  case DDSDXGI::BC2_UNorm:            return VkFormat::VK_FORMAT_BC2_UNORM_BLOCK;
  case DDSDXGI::BC2_UNorm_SRGB:       return VkFormat::VK_FORMAT_BC2_SRGB_BLOCK;
  //case DDSDXGI::BC3_Typeless:
  case DDSDXGI::BC3_UNorm:            return VkFormat::VK_FORMAT_BC3_UNORM_BLOCK;
  case DDSDXGI::BC3_UNorm_SRGB:       return VkFormat::VK_FORMAT_BC3_SRGB_BLOCK;
  //case DDSDXGI::BC4_Typeless:
  case DDSDXGI::BC4_UNorm:            return VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;
  case DDSDXGI::BC4_SNorm:            return VkFormat::VK_FORMAT_BC4_SNORM_BLOCK;
  //case DDSDXGI::BC5_Typeless:
  case DDSDXGI::BC5_UNorm:            return VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;
  case DDSDXGI::BC5_SNorm:            return VkFormat::VK_FORMAT_BC5_SNORM_BLOCK;
  case DDSDXGI::B5G6R5_UNorm:         return VkFormat::VK_FORMAT_B5G6R5_UNORM_PACK16;
  case DDSDXGI::B5G5R5A1_UNorm:       return VkFormat::VK_FORMAT_B5G5R5A1_UNORM_PACK16;
  case DDSDXGI::B8G8R8A8_UNorm:       return VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
  //case DDSDXGI::B8G8R8X8_UNorm:       return VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
  //case DDSDXGI::R10G10B10_XR_BIAS_A2_UNorm: 
  //case DDSDXGI::B8G8R8A8_Typeless:
  case DDSDXGI::B8G8R8A8_UNorm_SRGB:  return VkFormat::VK_FORMAT_B8G8R8A8_SRGB;
  //case DDSDXGI::B8G8R8X8_Typeless:
  //case DDSDXGI::B8G8R8X8_UNorm_SRGB:
  //case DDSDXGI::BC6H_Typeless:
  case DDSDXGI::BC6H_UF16:            return VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK;
  case DDSDXGI::BC6H_SF16:            return VkFormat::VK_FORMAT_BC6H_SFLOAT_BLOCK;
  //case DDSDXGI::BC7_Typeless:
  case DDSDXGI::BC7_UNorm:            return VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;
  case DDSDXGI::BC7_UNorm_SRGB:       return VkFormat::VK_FORMAT_BC7_SRGB_BLOCK;
  //case DDSDXGI::AYUV:
  //case DDSDXGI::Y410:
  //case DDSDXGI::Y416:
  //case DDSDXGI::NV12:
  //case DDSDXGI::P010:
  //case DDSDXGI::P016:
  //case DDSDXGI::YUV420_OPAQUE:
  //case DDSDXGI::YUY2:
  //case DDSDXGI::Y210:
  //case DDSDXGI::Y216:
  //case DDSDXGI::NV11:
  //case DDSDXGI::AI44:
  //case DDSDXGI::IA44:
  //case DDSDXGI::P8:
  //case DDSDXGI::A8P8:
  case DDSDXGI::B4G4R4A4_UNorm:       return VkFormat::VK_FORMAT_B4G4R4A4_UNORM_PACK16;
  //case DDSDXGI::P208:
  //case DDSDXGI::V208:
  //case DDSDXGI::V408:
  default:
    return VkFormat::VK_FORMAT_UNDEFINED;
  }
}

bool windowHandler::createTexture(vulkanTexture& outTexture, vulkanTexture::Setup const& inSetup)
{

#define CTPATHWARNHELPER(x) inSetup.m_Path.string().append(x)

  assert
  (
    outTexture.m_Image  == VK_NULL_HANDLE &&
    outTexture.m_Memory == VK_NULL_HANDLE &&
    outTexture.m_View   == VK_NULL_HANDLE
  );
  std::filesystem::directory_entry texDir{ inSetup.m_Path };
  if (false == texDir.exists() || texDir.is_directory())return false;

  tinyddsloader::DDSFile texFile; // bottom condition ensures texFile is loaded
  if (std::ifstream ifs{ texDir, std::ios_base::binary }; false == ifs.is_open() || false == tryTinyDDS(texFile.Load(ifs)))return false;
  
  VkFormat texFormat{ DXGIFormattoVkFormat(texFile.GetFormat()) };
  if (texFormat == VkFormat::VK_FORMAT_UNDEFINED)
  {
    printWarning(CTPATHWARNHELPER(" | Unsupported format for texture"sv), true);
    return false;
  }

  tinyddsloader::DDSFile::ImageData const* pImgData{ texFile.GetImageData() };
  if (nullptr == pImgData)
  {
    printWarning(CTPATHWARNHELPER(" | Could not get top face data"sv), true);
    return false;
  }
  outTexture.m_Extent.width  = pImgData->m_width;
  outTexture.m_Extent.height = pImgData->m_height;
  outTexture.m_Extent.depth  = pImgData->m_depth;

  { // create image
    VkImageCreateInfo imageCreateInfo
    {
      .sType{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO },
      .imageType  { VK_IMAGE_TYPE_2D },
      .format     { texFormat },
      .extent     { outTexture.m_Extent },
      .mipLevels  { texFile.GetMipCount() },
      .arrayLayers{ 1 },  // not sure how to extract if it does have
      .samples    { inSetup.m_Samples },
      .tiling     { inSetup.m_Tiling },
      .usage      { inSetup.m_Usage },
      .sharingMode{ VK_SHARING_MODE_EXCLUSIVE },
      .initialLayout{ VK_IMAGE_LAYOUT_UNDEFINED }
    };

    if (VkResult tmpRes{ vkCreateImage(m_pVKDevice->m_VKDevice, &imageCreateInfo, m_pVKInst->m_pVKAllocator, &outTexture.m_Image) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, CTPATHWARNHELPER(" | Failed to create VkImage"sv), true);
      destroyTexture(outTexture);
      return false;
    }
  }

  { // allocate image memory and bind memory to the previously created image
    VkMemoryRequirements texImageMemReqs;
    vkGetImageMemoryRequirements(m_pVKDevice->m_VKDevice, outTexture.m_Image, &texImageMemReqs);
    uint32_t memTypeIndex;
    if (bool tmpRes{ m_pVKDevice->getMemoryType(texImageMemReqs.memoryTypeBits, vulkanTexture::s_MemPropFlag_Sampler, memTypeIndex) }; false == tmpRes)
    {
      printWarning(CTPATHWARNHELPER(" | Failed to get memory type for image memory"sv), true);
      destroyTexture(outTexture);
      return false;
    }

    VkMemoryAllocateInfo texMemAllocInfo
    {
      .sType{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO },
      .allocationSize { texImageMemReqs.size },
      .memoryTypeIndex{ memTypeIndex }
    };

    if (VkResult tmpRes{ vkAllocateMemory(m_pVKDevice->m_VKDevice, &texMemAllocInfo, m_pVKInst->m_pVKAllocator, &outTexture.m_Memory) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, CTPATHWARNHELPER(" | Failed to allocate image memory"sv), true);
      destroyTexture(outTexture);
      return false;
    }

    vkBindImageMemory(m_pVKDevice->m_VKDevice, outTexture.m_Image, outTexture.m_Memory, 0);
  }

  { // copy texture from local memory to image memory
    uint32_t mipCount{ texFile.GetMipCount() };
    uint32_t stagingBufferReqSize{ 0 };
    for (uint32_t i{ 0 }; i < mipCount; ++i)
    {
      stagingBufferReqSize += texFile.GetImageData(i)->m_memSlicePitch;
    }

    vulkanBuffer stagingBuffer;
    if (false ==
      createBuffer
      (
        stagingBuffer,
        vulkanBuffer::Setup
        {
          .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Staging },
          .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Staging },
          .m_Count    { 1 },
          .m_ElemSize { stagingBufferReqSize }
        }
      ))
    {
      printWarning(CTPATHWARNHELPER(" | Failed to create staging buffer for image transfer"), true);
      destroyTexture(outTexture);
      return false;
    }

    void* dstData{ nullptr };
    if (VkResult tmpRes{ vkMapMemory(m_pVKDevice->m_VKDevice, stagingBuffer.m_BufferMemory, 0, VK_WHOLE_SIZE, 0, &dstData) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, CTPATHWARNHELPER(" | Failed to map staging buffer"sv), true);
      destroyBuffer(stagingBuffer);
      destroyTexture(outTexture);
      return false;
    }

    std::vector<VkBufferImageCopy> copyRegions;
    copyRegions.reserve(mipCount);
    for (uint32_t i{ 0 }, offset{ 0 }; i < mipCount; ++i)
    {
      tinyddsloader::DDSFile::ImageData const* pMipImgData{ texFile.GetImageData(i) };
      std::memcpy(reinterpret_cast<char*>(dstData) + offset, pMipImgData->m_mem, pMipImgData->m_memSlicePitch);
      copyRegions.emplace_back(VkBufferImageCopy{
        .bufferOffset{ offset },
        .bufferRowLength{ 0 },
        .bufferImageHeight{ 0 },
        .imageSubresource
        {
          .aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
          .mipLevel       { i },
          .baseArrayLayer { 0 },
          .layerCount     { 1 }
        },
        .imageOffset
        {
          .x{ 0 },
          .y{ 0 },
          .z{ 0 }
        },
        .imageExtent
        {
          .width  { pMipImgData->m_width },
          .height { pMipImgData->m_height },
          .depth  { pMipImgData->m_depth }
        }
      });
      offset += pMipImgData->m_memSlicePitch;
    }

    vkUnmapMemory(m_pVKDevice->m_VKDevice, stagingBuffer.m_BufferMemory);

    transitionImageLayout(outTexture.m_Image, texFormat, mipCount, true);

    if (VkCommandBuffer transferCmdBuffer{ beginOneTimeSubmitCommand() }; transferCmdBuffer != VK_NULL_HANDLE)
    {
      vkCmdCopyBufferToImage(transferCmdBuffer, stagingBuffer.m_Buffer, outTexture.m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
      endOneTimeSubmitCommand(transferCmdBuffer);
    }
    else
    {
      printWarning(CTPATHWARNHELPER(" | Failed to start transfer command queue"sv), true);
      destroyBuffer(stagingBuffer);
      destroyTexture(outTexture);
      return false;
    }

    transitionImageLayout(outTexture.m_Image, texFormat, mipCount, false);

    destroyBuffer(stagingBuffer);

  }

  { // create image view
    VkImageViewCreateInfo viewCreateInfo
    {
      .sType{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
      .image    { outTexture.m_Image },
      .viewType { VK_IMAGE_VIEW_TYPE_2D },
      .format   { texFormat },
      .components
      {
        .r{ VK_COMPONENT_SWIZZLE_IDENTITY },
        .g{ VK_COMPONENT_SWIZZLE_IDENTITY },
        .b{ VK_COMPONENT_SWIZZLE_IDENTITY },
        .a{ VK_COMPONENT_SWIZZLE_IDENTITY }
      },
      .subresourceRange
      {
        .aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
        .baseMipLevel   { 0 },
        .levelCount     { texFile.GetMipCount() },
        .baseArrayLayer { 0 },
        .layerCount     { 1 }
      }
    };

    if (VkResult tmpRes{ vkCreateImageView(m_pVKDevice->m_VKDevice, &viewCreateInfo, m_pVKInst->m_pVKAllocator, &outTexture.m_View) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, CTPATHWARNHELPER(" | failed to create image view"sv), true);
      destroyTexture(outTexture);
      return false;
    }
  }

  { // create sampler
    VkSamplerCreateInfo samplerCreateInfo
    {
      .sType{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO },
      .magFilter        { VK_FILTER_LINEAR },
      .minFilter        { VK_FILTER_LINEAR },
      .addressModeU     { inSetup.m_AddressModeU },
      .addressModeV     { inSetup.m_AddressModeV },
      .addressModeW     { inSetup.m_AddressModeW },
      .mipLodBias       { 0.0f },
      .anisotropyEnable { VK_TRUE },
      .maxAnisotropy    { std::min(16.0f, m_pVKDevice->m_VKPhysicalDeviceProperties.limits.maxSamplerAnisotropy) },
      .compareEnable    { VK_FALSE },
      .compareOp        { VK_COMPARE_OP_ALWAYS },
      .minLod           { 0.0f },
      .maxLod           { static_cast<float>(texFile.GetMipCount()) },
      .borderColor      { VK_BORDER_COLOR_INT_OPAQUE_BLACK },
      .unnormalizedCoordinates{ VK_FALSE }
    };
    if (VkResult tmpRes{ vkCreateSampler(m_pVKDevice->m_VKDevice, &samplerCreateInfo, m_pVKInst->m_pVKAllocator, &outTexture.m_Sampler) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, CTPATHWARNHELPER(" | failed to create sampler"sv), true);
      destroyTexture(outTexture);
      return false;
    }
  }

  return true;
#undef CTPATHWARNHELPER
}

void windowHandler::transitionImageLayout(VkImage image, VkFormat format, uint32_t mipLevels, bool isTransferStart)
{
  if (VkCommandBuffer CBuf{ beginOneTimeSubmitCommand(!isTransferStart) }; CBuf != VK_NULL_HANDLE)
  {
    VkImageMemoryBarrier imgBarrier
    {
      .sType{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER },
      .srcAccessMask      { static_cast<VkAccessFlags>(isTransferStart ? VK_ACCESS_NONE_KHR : VK_ACCESS_TRANSFER_WRITE_BIT) },
      .dstAccessMask      { static_cast<VkAccessFlags>(isTransferStart ? VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_SHADER_READ_BIT) },
      .oldLayout          { isTransferStart ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
      .newLayout          { isTransferStart ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
      .srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
      .dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
      .image              { image },
      .subresourceRange
      {
        .aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
        .baseMipLevel   { 0 },
        .levelCount     { mipLevels },
        .baseArrayLayer { 0 },
        .layerCount     { 1 }
      }
    };

    vkCmdPipelineBarrier
    (
      CBuf,
      isTransferStart ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT,
      isTransferStart ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &imgBarrier
    );

    endOneTimeSubmitCommand(CBuf, !isTransferStart);
  }
}

void windowHandler::destroyTexture(vulkanTexture& inTexture)
{
  if (inTexture.m_Sampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(m_pVKDevice->m_VKDevice, inTexture.m_Sampler, m_pVKInst->m_pVKAllocator);
    inTexture.m_Sampler = VK_NULL_HANDLE;
  }
  if (inTexture.m_View != VK_NULL_HANDLE)
  {
    vkDestroyImageView(m_pVKDevice->m_VKDevice, inTexture.m_View, m_pVKInst->m_pVKAllocator);
    inTexture.m_View = VK_NULL_HANDLE;
  }
  if (inTexture.m_Memory != VK_NULL_HANDLE)
  {
    vkFreeMemory(m_pVKDevice->m_VKDevice, inTexture.m_Memory, m_pVKInst->m_pVKAllocator);
    inTexture.m_Memory = VK_NULL_HANDLE;
  }
  if (inTexture.m_Image != VK_NULL_HANDLE)
  {
    vkDestroyImage(m_pVKDevice->m_VKDevice, inTexture.m_Image, m_pVKInst->m_pVKAllocator);
    inTexture.m_Image = VK_NULL_HANDLE;
  }
  inTexture.m_Extent.depth = inTexture.m_Extent.height = inTexture.m_Extent.width = 0;
}

#pragma warning (default : 26812)// reenable unscoped enum warning
