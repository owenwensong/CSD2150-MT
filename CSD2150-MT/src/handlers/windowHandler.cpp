/*!*****************************************************************************
 * @file    graphicsHandler_vulkan.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    12 FEB 2022
 * @brief   This is the implementation for the graphicsHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/printWarnings.h>
#include <handlers/windowHandler.h>
#include <utility/vertices.h>
#include <iostream>
#include <iterator>
#include <fstream>


// *************************************************** WINDOW HANDLER CLASS ****

windowHandler::windowHandler(size_t flagOptions) :
  m_pVKInst
  {
    std::make_shared<vulkanInstance>
    (
      flagOptions & flagDebugLayer ? true : false,
      flagOptions & flagRenderDocLayer ? true : false
    )
  },
  m_pVKDevice{ std::make_shared<vulkanDevice>(m_pVKInst) },
  bDebugPrint{ flagOptions & flagDebugPrint ? true : false }
{
  if (bDebugPrint)
  {
    std::cout << "graphicsHandler instance created! \nvulkanInstance status: "sv
              << (m_pVKInst && m_pVKInst->OK() ? "OK"sv : "BAD"sv)
              << "\nvulkanDevice status: "sv
              << (m_pVKDevice && m_pVKDevice->OK() ? "OK"sv : "BAD"sv)
              << std::endl;
  }

}

windowHandler::~windowHandler()
{
  if (bDebugPrint)
  {
    std::cout << "graphicsHandler instance destruct!"sv << std::endl;
  }
}

bool windowHandler::OK() const noexcept
{
  return
  {
    (m_pVKInst.get() != nullptr && m_pVKInst->OK()) &&
    (m_pVKDevice.get() != nullptr && m_pVKDevice->OK())
  };
}

bool windowHandler::processInputEvents()
{
  for (MSG msg; PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE); DispatchMessage(&msg))
  {
    if (msg.message == WM_QUIT)return false;
  }
  return true;
}

std::unique_ptr<vulkanWindow> windowHandler::createWindow(windowSetup const& Setup)
{
  return std::make_unique<vulkanWindow>(m_pVKDevice, Setup);
}

VkShaderModule windowHandler::createShaderModule(const char* relPath)
{
  if (std::ifstream ifs{ relPath, std::ios::binary }; ifs.is_open())
  {
    return createShaderModule
    (
      std::vector<char>
      {
        std::istreambuf_iterator<char>{ ifs },
        std::istreambuf_iterator<char>{ /* EOF */ }
      }
    );
  }
  printWarning("Invalid relative file provided for shader"sv, true);
  return VK_NULL_HANDLE;
}

VkShaderModule windowHandler::createShaderModule(std::vector<char> const& code)
{
  VkShaderModule retval{ VK_NULL_HANDLE };		// assuming NRVO
  VkShaderModuleCreateInfo CreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO },
    .codeSize{ code.size() },
    .pCode{ reinterpret_cast<uint32_t const*>(code.data()) }
  };
  if (VkResult tmpRes{ vkCreateShaderModule(m_pVKDevice->m_VKDevice, &CreateInfo, m_pVKInst->m_pVKAllocator, &retval) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "Shader Module Creation failed"sv, true);
  }
  return retval;
}

void windowHandler::destroyShaderModule(VkShaderModule& shaderModule)
{
  if (shaderModule == VK_NULL_HANDLE)return;
  vkDestroyShaderModule(m_pVKDevice->m_VKDevice, shaderModule, m_pVKInst->m_pVKAllocator);
  shaderModule = VK_NULL_HANDLE;
}

VkPipelineLayout windowHandler::createPipelineLayout(VkPipelineLayoutCreateInfo const& CreateInfo)
{
  VkPipelineLayout retval{ VK_NULL_HANDLE };
  if (VkResult tmpRes{ vkCreatePipelineLayout(m_pVKDevice->m_VKDevice, &CreateInfo, m_pVKInst->m_pVKAllocator, &retval) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "Failed to create pipeline layout"sv, true);
  }
  return retval;
}

void windowHandler::destroyPipelineLayout(VkPipelineLayout& pipelineLayout)
{
  if (pipelineLayout == VK_NULL_HANDLE)return;
  vkDestroyPipelineLayout(m_pVKDevice->m_VKDevice, pipelineLayout, m_pVKInst->m_pVKAllocator);
  pipelineLayout = VK_NULL_HANDLE;
}

VkCommandBuffer windowHandler::beginOneTimeSubmitCommand(bool useMainCommandPool)
{
  VkCommandPool cmdPool{ useMainCommandPool ? m_pVKDevice->m_TransferCommandSpecialPool : m_pVKDevice->m_TransferCommandPool };
  VkCommandBufferAllocateInfo AllocInfo
  {
    .sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
    .commandPool        { cmdPool },
    .level              { VK_COMMAND_BUFFER_LEVEL_PRIMARY },
    .commandBufferCount { 1 }
  };
  VkCommandBuffer retval;
  if (VkResult tmpRes{ vkAllocateCommandBuffers(m_pVKDevice->m_VKDevice, &AllocInfo, &retval) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "failed to allocate one time submit command buffer"sv, true);
    return VK_NULL_HANDLE;
  }

  {
    VkCommandBufferBeginInfo BeginInfo
    {
      .sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
      .flags{ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT }
    };
    if (VkResult tmpRes{ vkBeginCommandBuffer(retval, &BeginInfo) }; tmpRes != VK_SUCCESS)
    {
      vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, cmdPool, 1, &retval);
      printVKWarning(tmpRes, "failed to begin one time submit command buffer"sv, true);
      return VK_NULL_HANDLE;
    }
  }

  return retval;
}

void windowHandler::endOneTimeSubmitCommand(VkCommandBuffer toEnd, bool useMainCommandPool)
{
  VkCommandPool cmdPool{ useMainCommandPool ? m_pVKDevice->m_TransferCommandSpecialPool : m_pVKDevice->m_TransferCommandPool };
  auto& lockableQueue{ useMainCommandPool ? m_pVKDevice->m_VKMainQueue : m_pVKDevice->m_VKTransferQueue };
  if (VkResult tmpRes{ vkEndCommandBuffer(toEnd) }; tmpRes != VK_SUCCESS)
  {
    vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, cmdPool, 1, &toEnd);
    printVKWarning(tmpRes, "failed to end transfer command buffer"sv, true);
    return;
  }
  VkSubmitInfo SubmitInfo
  {
    .sType{ VK_STRUCTURE_TYPE_SUBMIT_INFO },
    .commandBufferCount { 1 },
    .pCommandBuffers    { &toEnd }
  };
  std::scoped_lock Lk{ lockableQueue };
  if (VkResult tmpRes{ vkQueueSubmit(lockableQueue.get(), 1, &SubmitInfo, VK_NULL_HANDLE) }; tmpRes != VK_SUCCESS)
  {
    vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, cmdPool, 1, &toEnd);
    printVKWarning(tmpRes, "failed to submit transfer queue"sv, true);
    return;
  }
  if (VkResult tmpRes{ vkQueueWaitIdle(lockableQueue.get()) }; tmpRes != VK_SUCCESS)
  {
    vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, cmdPool, 1, &toEnd);
    printVKWarning(tmpRes, "failed to wait for transfer queue"sv, true);
    return;
  }
  vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, cmdPool, 1, &toEnd);
}

bool windowHandler::writeToBuffer(vulkanBuffer& dstBuffer, std::vector<void*> const& srcs, std::vector<VkDeviceSize> const& srcLens)
{ 
  uint32_t totalSrcLen{ 0 };
  for (VkDeviceSize x : srcLens)totalSrcLen += static_cast<uint32_t>(x);
  assert(srcs.size() == srcLens.size());
  assert(totalSrcLen <= dstBuffer.m_Settings.m_Count * dstBuffer.m_Settings.m_ElemSize);

  vulkanBuffer stagingBuffer;
  if (false == 
    createBuffer
    (
      stagingBuffer, 
      vulkanBuffer::Setup
      {
        .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Staging },
        .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Staging },
        .m_Count{ totalSrcLen },
        .m_ElemSize{ 1 }
      }
    ))
  {
    printWarning("failed to create staging buffer"sv, true);
    return false;
  }

  void* dstData{ nullptr };
  if (VkResult tmpRes{ vkMapMemory(m_pVKDevice->m_VKDevice, stagingBuffer.m_BufferMemory, 0, VK_WHOLE_SIZE, 0, &dstData) }; tmpRes != VK_SUCCESS)
  {
    destroyBuffer(stagingBuffer);
    printVKWarning(tmpRes, "Failed to map staging buffer"sv, true);
    return false;
  }
  for (size_t i{ 0 }, t{ srcs.size() }; i < t; ++i)
  {
    std::memcpy(dstData, srcs[i], static_cast<size_t>(srcLens[i]));
    dstData = reinterpret_cast<char*>(dstData) + srcLens[i];
  }
  vkUnmapMemory(m_pVKDevice->m_VKDevice, stagingBuffer.m_BufferMemory);

  bool retval{ copyBuffer(dstBuffer, stagingBuffer, totalSrcLen) };
  destroyBuffer(stagingBuffer);
  return retval;
}

bool windowHandler::copyBuffer(vulkanBuffer& dstBuffer, vulkanBuffer& srcBuffer, VkDeviceSize cpySize)
{
  if (VkCommandBuffer transferCmdBuffer{ beginOneTimeSubmitCommand() }; transferCmdBuffer != VK_NULL_HANDLE)
  {
    VkBufferCopy copyRegion
    {
      .srcOffset{ 0 },
      .dstOffset{ 0 },
      .size{ cpySize }
    };
    vkCmdCopyBuffer(transferCmdBuffer, srcBuffer.m_Buffer, dstBuffer.m_Buffer, 1, &copyRegion);
    endOneTimeSubmitCommand(transferCmdBuffer);
  }
  return true;
}

bool windowHandler::createBuffer(vulkanBuffer& outBuffer, vulkanBuffer::Setup const& inSetup)
{
  destroyBuffer(outBuffer);

  if (uint32_t tmpSize{ inSetup.m_Count * inSetup.m_ElemSize }; tmpSize == 0)
  {
    printWarning("Trying to make buffer of size 0"sv, true);
    return false;
  }
  else
  {
    VkBufferCreateInfo CreateInfo
    {
      .sType{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO },
      .size { tmpSize },
      .usage{ inSetup.m_BufferUsage },
      .sharingMode{ VK_SHARING_MODE_EXCLUSIVE }
    };
    if (VkResult tmpRes{ vkCreateBuffer(m_pVKDevice->m_VKDevice, &CreateInfo, m_pVKInst->m_pVKAllocator, &outBuffer.m_Buffer) }; tmpRes != VK_SUCCESS)
    {
      printVKWarning(tmpRes, "Failed to create a buffer"sv, true);
      return false;
    }
  }

  VkMemoryRequirements memReq;
  vkGetBufferMemoryRequirements(m_pVKDevice->m_VKDevice, outBuffer.m_Buffer, &memReq);

  if (uint32_t memTypeIndex; m_pVKDevice->getMemoryType(memReq.memoryTypeBits, inSetup.m_MemPropFlag, memTypeIndex))
  {
    VkMemoryAllocateInfo AllocInfo
    {
      .sType{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO },
      .allocationSize { memReq.size },
      .memoryTypeIndex{ memTypeIndex },
    };
    if (VkResult tmpRes{ vkAllocateMemory(m_pVKDevice->m_VKDevice, &AllocInfo, m_pVKInst->m_pVKAllocator, &outBuffer.m_BufferMemory) }; tmpRes != VK_SUCCESS)
    {
      destroyBuffer(outBuffer);
      printVKWarning(tmpRes, "Failed to allocate buffer memory"sv, true);
      return false;
    }
  }
  else
  {
    destroyBuffer(outBuffer);
    printWarning("Failed to find memory type for buffer"sv, true);
    return false;
  }

  if (VkResult tmpRes{ vkBindBufferMemory(m_pVKDevice->m_VKDevice, outBuffer.m_Buffer, outBuffer.m_BufferMemory, 0) }; tmpRes != VK_SUCCESS)
  {
    destroyBuffer(outBuffer);
    printVKWarning(tmpRes, "Failed to bind buffer memory"sv, true);
    return false;
  }
  outBuffer.m_Settings = inSetup;
  return true;
}

void windowHandler::destroyBuffer(vulkanBuffer& inBuffer)
{
  inBuffer.m_Settings = vulkanBuffer::Setup{};
  if (inBuffer.m_Buffer != VK_NULL_HANDLE)
  {
    m_pVKDevice->waitForDeviceIdle();
    vkDestroyBuffer(m_pVKDevice->m_VKDevice, inBuffer.m_Buffer, m_pVKInst->m_pVKAllocator);
    inBuffer.m_Buffer = VK_NULL_HANDLE;
  }
  if (inBuffer.m_BufferMemory != VK_NULL_HANDLE)
  {
    m_pVKDevice->waitForDeviceIdle();
    vkFreeMemory(m_pVKDevice->m_VKDevice, inBuffer.m_BufferMemory, m_pVKInst->m_pVKAllocator);
    inBuffer.m_BufferMemory = VK_NULL_HANDLE;
  }
}

bool windowHandler::setupVertexInputInfo(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup)
{
  outPipeline.m_BindingDescription[0] = VkVertexInputBindingDescription
  {
    .binding  { 0 },
    // stride dependant on mode
    .inputRate{ VK_VERTEX_INPUT_RATE_VERTEX }// no instancing yet of course
  };
  
  switch (inSetup.m_VertexBindingMode)
  {
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XY_UV_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_2D_UV));
    outPipeline.m_AttributeDescription.reserve(2);
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 0 },  // layout location 0
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32_SFLOAT },
      .offset   { 0 }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 1 },  // layout location 1
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32_SFLOAT },
      .offset   { offsetof(VTX_2D_UV, m_Tex) }
    });
    break;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XY_RGB_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_2D_RGB));
    outPipeline.m_AttributeDescription.reserve(2);
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 0 },  // layout location 0
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32_SFLOAT },
      .offset   { 0 }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 1 },  // layout location 1
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { offsetof(VTX_2D_RGB, m_Col) }
    });
    break;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XY_RGBA_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_2D_RGBA));
    outPipeline.m_AttributeDescription.reserve(2);
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 0 },  // layout location 0
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32_SFLOAT },
      .offset   { 0 }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 1 },  // layout location 1
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32A32_SFLOAT },
      .offset   { offsetof(VTX_2D_RGBA, m_Col) }
    });
    break;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_UV_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_3D_UV));
    outPipeline.m_AttributeDescription.reserve(2);
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 0 },  // layout location 0
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { 0 }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 1 },  // layout location 1
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32_SFLOAT },
      .offset   { offsetof(VTX_3D_UV, m_Tex) }
    });
    break;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_UV_NML_TAN_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_3D_UV_NML_TAN));
    outPipeline.m_AttributeDescription.reserve(4);
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 0 },  // layout location 0
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { 0 }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 1 },  // layout location 1
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32_SFLOAT },
      .offset   { offsetof(VTX_3D_UV_NML_TAN, m_Tex) }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 2 },  // layout location 2
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { offsetof(VTX_3D_UV_NML_TAN, m_Nml) }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 3 },  // layout location 3
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { offsetof(VTX_3D_UV_NML_TAN, m_Tan) }
    });
    break;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_RGB_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_3D_RGB));
    outPipeline.m_AttributeDescription.reserve(2);
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 0 },  // layout location 0
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { 0 }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 1 },  // layout location 1
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { offsetof(VTX_3D_RGB, m_Col) }
    });
    break;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_RGBA_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_3D_RGBA));
    outPipeline.m_AttributeDescription.reserve(2);
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 0 },  // layout location 0
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32_SFLOAT },
      .offset   { 0 }
    });
    outPipeline.m_AttributeDescription.emplace_back(VkVertexInputAttributeDescription{
      .location { 1 },  // layout location 1
      .binding  { 0 },  // bound buffer 0 (SOA)
      .format   { VK_FORMAT_R32G32B32A32_SFLOAT },
      .offset   { offsetof(VTX_3D_RGBA, m_Col) }
    });
    break;
  default:
    printWarning("UNKNOWN VERTEX BINDING MODE PROVIDED"sv);
    return false;
  }
  outPipeline.m_VertexInputInfo = VkPipelineVertexInputStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO },
    .vertexBindingDescriptionCount	{ static_cast<uint32_t>(outPipeline.m_BindingDescription.size()) },
    .pVertexBindingDescriptions			{ outPipeline.m_BindingDescription.data() },
    .vertexAttributeDescriptionCount{ static_cast<uint32_t>(outPipeline.m_AttributeDescription.size()) },
    .pVertexAttributeDescriptions		{ outPipeline.m_AttributeDescription.data() }
  };
  return true;
}

// *****************************************************************************
