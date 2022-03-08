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

bool windowHandler::createPipelineInfo(vulkanPipeline& outPipeline, vulkanPipeline::setup const& inSetup)
{

  destroyPipelineInfo(outPipeline); // make sure it's fresh boi

  if (inSetup.m_VertexBindingMode == vulkanPipeline::E_VERTEX_BINDING_MODE::UNDEFINED)
  {
    printWarning("Cannot create the pipeline layout. Vertex binding mode not defined."sv, true);
    return false;
  }

  outPipeline.m_ShaderVert = createShaderModule(inSetup.m_PathShaderVert.data());
  if (outPipeline.m_ShaderVert == VK_NULL_HANDLE)
  {
    std::string msg{ "Cannot create pipeline layout. Vertex shader failed to create from: " };
    msg.append(inSetup.m_PathShaderVert);
    printWarning(msg, true);
    destroyPipelineInfo(outPipeline);
    return false;
  }
  outPipeline.m_ShaderFrag = createShaderModule(inSetup.m_PathShaderFrag.data());
  if (outPipeline.m_ShaderFrag == VK_NULL_HANDLE)
  {
    std::string msg{ "Cannot create pipeline layout. Fragment shader failed to create from: " };
    msg.append(inSetup.m_PathShaderFrag);
    printWarning(msg, true);
    destroyPipelineInfo(outPipeline);
    return false;
  }
  { // local scope for VkPipelineLayout
    VkPipelineLayoutCreateInfo CreateInfo
    {
      .sType{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO },
      .setLayoutCount         { 0 },      // future for descriptor sets
      .pSetLayouts            { nullptr },// future for descriptor sets
      .pushConstantRangeCount { 0 },      // default to 0
      .pPushConstantRanges    { nullptr } // default to nullptr
    };
    if (inSetup.m_PushConstantRangeVert.size != 0)
    {
      CreateInfo.pPushConstantRanges = &inSetup.m_PushConstantRangeVert;
      CreateInfo.pushConstantRangeCount = (inSetup.m_PushConstantRangeFrag.size ? 2 : 1);
    }
    else if (inSetup.m_PushConstantRangeFrag.size != 0)
    {
      CreateInfo.pPushConstantRanges = &inSetup.m_PushConstantRangeFrag;
      CreateInfo.pushConstantRangeCount = 1;
    }
    outPipeline.m_PipelineLayout = createPipelineLayout(CreateInfo);
    if (outPipeline.m_PipelineLayout == VK_NULL_HANDLE)
    {
      printWarning("Cannot create pipeline layout. PipelineLayout creation failed", true);
      destroyPipelineInfo(outPipeline);
      return false;
    }
  }

  outPipeline.m_ShaderStages[0] = VkPipelineShaderStageCreateInfo
  {
    .sType { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO },
    .stage { VK_SHADER_STAGE_VERTEX_BIT },
    .module{ outPipeline.m_ShaderVert },
    .pName { "main" }
  };
  outPipeline.m_ShaderStages[1] = VkPipelineShaderStageCreateInfo
  {
    .sType { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO },
    .stage { VK_SHADER_STAGE_FRAGMENT_BIT },
    .module{ outPipeline.m_ShaderFrag },
    .pName { "main" }
  };

  if (false == setupVertexInputInfo(outPipeline, inSetup))
  {
    destroyPipelineInfo(outPipeline);
    printWarning("could not create pipeline info, failed to setup vertex input info."sv, true);
    return false;
  }

  outPipeline.m_InputAssembly = VkPipelineInputAssemblyStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO },
    .topology								{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
    .primitiveRestartEnable	{ VK_FALSE }
  };

  outPipeline.m_Rasterizer = VkPipelineRasterizationStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO },
    .depthClampEnable				{ VK_FALSE },	// clamp instead of discarding stuff outside the near/far planes
    .rasterizerDiscardEnable{ VK_FALSE },
    .polygonMode						{ VK_POLYGON_MODE_FILL },
    .cullMode								{ VK_CULL_MODE_BACK_BIT },	// back face culling
    .frontFace							{ VK_FRONT_FACE_COUNTER_CLOCKWISE },
    .depthBiasEnable				{ VK_FALSE },
    .depthBiasConstantFactor{ 0.0f },
    .depthBiasClamp					{ 0.0f },
    .depthBiasSlopeFactor		{ 0.0f },
    .lineWidth							{ 1.0f }
  };

  outPipeline.m_Multisampling = VkPipelineMultisampleStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO },
    .rasterizationSamples		{ VK_SAMPLE_COUNT_1_BIT },
    .sampleShadingEnable		{ VK_FALSE },
    .minSampleShading				{ 1.0f },
    .pSampleMask						{ nullptr },
    .alphaToCoverageEnable	{ VK_FALSE },
    .alphaToOneEnable				{ VK_FALSE }
  };

  outPipeline.m_DepthStencilState = VkPipelineDepthStencilStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO },
    .pNext{ nullptr },
    .flags{ 0 },
    .depthTestEnable{ VK_TRUE },		// make this an option next time?
    .depthWriteEnable{ VK_TRUE },		// make this an option next time?
    .depthCompareOp{ VK_COMPARE_OP_LESS_OR_EQUAL },
    .depthBoundsTestEnable{ VK_TRUE },	// make this an option next time?
    .stencilTestEnable{ VK_TRUE },			// make this an option next time?
    .front	// NO IDEA IF ANY OF THESE ARE CORRECT
    {
      .failOp{ VK_STENCIL_OP_KEEP },
      .passOp{ VK_STENCIL_OP_KEEP },
      .depthFailOp{ VK_STENCIL_OP_KEEP },
      .compareOp{ VK_COMPARE_OP_ALWAYS },
      .compareMask{ ~0u },
      .reference{ 0u }// is an integer reference value that is used in the unsigned stencil comparison
    },
    .back	// NO IDEA IF ANY OF THESE ARE CORRECT
    {
      .failOp{ VK_STENCIL_OP_KEEP },
      .passOp{ VK_STENCIL_OP_KEEP },
      .depthFailOp{ VK_STENCIL_OP_KEEP },
      .compareOp{ VK_COMPARE_OP_ALWAYS },
      .compareMask{ ~0u },
      .reference{ 0u }// is an integer reference value that is used in the unsigned stencil comparison
    },
    .minDepthBounds{ 0.0f },
    .maxDepthBounds{ 1.0f }
  };

  outPipeline.m_ColorBlendAttachment = VkPipelineColorBlendAttachmentState
  {
    .blendEnable				{ VK_TRUE },		// should it be false?
    .srcColorBlendFactor{ VK_BLEND_FACTOR_SRC_ALPHA },
    .dstColorBlendFactor{ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA },
    .colorBlendOp				{ VK_BLEND_OP_ADD },
    .srcAlphaBlendFactor{ VK_BLEND_FACTOR_SRC_ALPHA },
    .dstAlphaBlendFactor{ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA },
    .alphaBlendOp{},
    .colorWriteMask
    {
      VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT
    }
  };

  outPipeline.m_ColorBlending = VkPipelineColorBlendStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO },
    .logicOpEnable	{ VK_TRUE },
    .logicOp				{ VK_LOGIC_OP_COPY },
    .attachmentCount{ 1 },
    .pAttachments		{ &outPipeline.m_ColorBlendAttachment },
    .blendConstants { 0.0f, 0.0f, 0.0f, 0.0f }	// ????
  };

  outPipeline.m_DynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
  outPipeline.m_DynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

  outPipeline.m_DynamicStateCreateInfo = VkPipelineDynamicStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO },
    .dynamicStateCount	{ static_cast<uint32_t>(outPipeline.m_DynamicStates.size()) },
    .pDynamicStates			{ outPipeline.m_DynamicStates.data() }
  };

  return true;
}

void windowHandler::destroyPipelineInfo(vulkanPipeline& inPipeline)
{
  destroyPipelineLayout(inPipeline.m_PipelineLayout);
  destroyShaderModule(inPipeline.m_ShaderFrag);
  destroyShaderModule(inPipeline.m_ShaderVert);
}

bool windowHandler::writeToBuffer(vulkanBuffer& dstBuffer, void* srcData, VkDeviceSize srcLen)
{
  assert(srcData);  // not much time to think, just assert it
  assert(srcLen <= dstBuffer.m_Settings.m_Count * dstBuffer.m_Settings.m_ElemSize);
  
  vulkanBuffer stagingBuffer;
  if (false == 
    createBuffer
    (
      stagingBuffer, 
      vulkanBuffer::Setup
      {
        .m_BufferUsage{ vulkanBuffer::s_BufferUsage_Staging },
        .m_MemPropFlag{ vulkanBuffer::s_MemPropFlag_Staging },
        .m_Count{ static_cast<uint32_t>(srcLen) },
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
  std::memcpy(dstData, srcData, srcLen);
  vkUnmapMemory(m_pVKDevice->m_VKDevice, stagingBuffer.m_BufferMemory);

  bool retval{ copyBuffer(dstBuffer, stagingBuffer, srcLen) };
  destroyBuffer(stagingBuffer);
  return retval;
}

bool windowHandler::copyBuffer(vulkanBuffer& dstBuffer, vulkanBuffer& srcBuffer, VkDeviceSize cpySize)
{
  VkCommandBufferAllocateInfo AllocInfo
  {
    .sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
    .commandPool        { m_pVKDevice->m_TransferCommandPool },
    .level              { VK_COMMAND_BUFFER_LEVEL_PRIMARY },
    .commandBufferCount { 1 }
  };
  VkCommandBuffer transferCmdBuffer;
  if (VkResult tmpRes{ vkAllocateCommandBuffers(m_pVKDevice->m_VKDevice, &AllocInfo, &transferCmdBuffer) }; tmpRes != VK_SUCCESS)
  {
    printVKWarning(tmpRes, "failed to allocate transfer command buffer"sv, true);
    return false;
  }

  {
    VkCommandBufferBeginInfo BeginInfo
    {
      .sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
      .flags{ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT }
    };
    if (VkResult tmpRes{ vkBeginCommandBuffer(transferCmdBuffer, &BeginInfo) }; tmpRes != VK_SUCCESS)
    {
      vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, m_pVKDevice->m_TransferCommandPool, 1, &transferCmdBuffer);
      printVKWarning(tmpRes, "failed to begin transfer command buffer"sv, true);
      return false;
    }
  }

  {
    VkBufferCopy copyRegion
    {
      .srcOffset{ 0 },
      .dstOffset{ 0 },
      .size{ cpySize }
    };
    vkCmdCopyBuffer(transferCmdBuffer, srcBuffer.m_Buffer, dstBuffer.m_Buffer, 1, &copyRegion);
    if (VkResult tmpRes{ vkEndCommandBuffer(transferCmdBuffer) }; tmpRes != VK_SUCCESS)
    {
      vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, m_pVKDevice->m_TransferCommandPool, 1, &transferCmdBuffer);
      printVKWarning(tmpRes, "failed to end transfer command buffer"sv, true);
      return false;
    }
  }

  {
    VkSubmitInfo SubmitInfo
    {
      .sType{ VK_STRUCTURE_TYPE_SUBMIT_INFO },
      .commandBufferCount { 1 },
      .pCommandBuffers    { &transferCmdBuffer }
    };
    std::scoped_lock Lk{ m_pVKDevice->m_VKTransferQueue };
    if (VkResult tmpRes{ vkQueueSubmit(m_pVKDevice->m_VKTransferQueue.get(), 1, &SubmitInfo, VK_NULL_HANDLE) }; tmpRes != VK_SUCCESS)
    {
      vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, m_pVKDevice->m_TransferCommandPool, 1, &transferCmdBuffer);
      printVKWarning(tmpRes, "failed to submit transfer queue"sv, true);
      return false;
    }
    if (VkResult tmpRes{ vkQueueWaitIdle(m_pVKDevice->m_VKTransferQueue.get()) }; tmpRes != VK_SUCCESS)
    {
      vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, m_pVKDevice->m_TransferCommandPool, 1, &transferCmdBuffer);
      printVKWarning(tmpRes, "failed to wait for transfer queue"sv, true);
      return false;
    }
  }

  vkFreeCommandBuffers(m_pVKDevice->m_VKDevice, m_pVKDevice->m_TransferCommandPool, 1, &transferCmdBuffer);
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
  outPipeline.m_AttributeDescription[0] = VkVertexInputAttributeDescription
  {
    .location { 0 },  // layout location 0
    .binding  { 0 },  // bound buffer 0 (SOA)
    // format dependant on mode
    .offset   { 0 }   // assume offset always start at 0, not using struct def
  };
  outPipeline.m_AttributeDescription[1] = VkVertexInputAttributeDescription
  {
    .location { 1 },  // layout location 1
    .binding  { 0 },  // bound buffer 0 (SOA)
    // format dependant on mode
    // assume offset based on previous attribute size
  };
  outPipeline.m_VertexInputInfo = VkPipelineVertexInputStateCreateInfo
  {
    .sType{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO },
    .vertexBindingDescriptionCount	{ 1 },
    .pVertexBindingDescriptions			{ outPipeline.m_BindingDescription.data() },
    .vertexAttributeDescriptionCount{ 2 },
    .pVertexAttributeDescriptions		{ outPipeline.m_AttributeDescription.data() }
  };
  switch (inSetup.m_VertexBindingMode)
  {
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XY_RGB_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_2D_RGB));
    outPipeline.m_AttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
    outPipeline.m_AttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    outPipeline.m_AttributeDescription[1].offset = offsetof(VTX_2D_RGB, m_Col);
    return true;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XY_RGBA_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_2D_RGBA));
    outPipeline.m_AttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
    outPipeline.m_AttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    outPipeline.m_AttributeDescription[1].offset = offsetof(VTX_2D_RGBA, m_Col);
    return true;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_RGB_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_3D_RGB));
    outPipeline.m_AttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    outPipeline.m_AttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    outPipeline.m_AttributeDescription[1].offset = offsetof(VTX_3D_RGB, m_Col);;
    return true;
  case vulkanPipeline::E_VERTEX_BINDING_MODE::AOS_XYZ_RGBA_F32:
    outPipeline.m_BindingDescription[0].stride = static_cast<uint32_t>(sizeof(VTX_3D_RGBA));
    outPipeline.m_AttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    outPipeline.m_AttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    outPipeline.m_AttributeDescription[1].offset = offsetof(VTX_3D_RGBA, m_Col);;
    return true;
  default:
    printWarning("UNKNOWN VERTEX BINDING MODE PROVIDED"sv);
    return false;
  }

  
}

// *****************************************************************************
