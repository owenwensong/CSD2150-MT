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
				(m_pVKInst.get()		!= nullptr && m_pVKInst->OK()		) &&
				(m_pVKDevice.get()	!= nullptr && m_pVKDevice->OK()	)
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
				printVKWarning(tmpRes, ""sv, true);
		}
		return retval;
}

void windowHandler::destroyPipelineLayout(VkPipelineLayout& pipelineLayout)
{
		if (pipelineLayout == VK_NULL_HANDLE)return;
		vkDestroyPipelineLayout(m_pVKDevice->m_VKDevice, pipelineLayout, m_pVKInst->m_pVKAllocator);
		pipelineLayout = VK_NULL_HANDLE;
}

bool windowHandler::createPipelineInfo(vulkanPipeline& outPipeline, VkShaderModule const& vertShader, VkShaderModule const& fragShader)
{
		if (vertShader == VK_NULL_HANDLE || fragShader == VK_NULL_HANDLE)
		{
				printWarning("Cannot create pipeline layout, one or more of the shader modules provided are invalid"sv, true);
				return false;
		}
		outPipeline.m_ShaderStages[0] = VkPipelineShaderStageCreateInfo
		{
				.sType { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO },
				.stage { VK_SHADER_STAGE_VERTEX_BIT },
				.module{ vertShader },
				.pName { "main" }
		};
		outPipeline.m_ShaderStages[1] = VkPipelineShaderStageCreateInfo
		{
				.sType { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO },
				.stage { VK_SHADER_STAGE_FRAGMENT_BIT },
				.module{ fragShader },
				.pName { "main" }
		};

		outPipeline.m_VertexInputInfo = VkPipelineVertexInputStateCreateInfo
		{
				.sType{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO },
				.vertexBindingDescriptionCount	{ 0 },
				.pVertexBindingDescriptions			{ nullptr },
				.vertexAttributeDescriptionCount{ 0 },
				.pVertexAttributeDescriptions		{ nullptr }
		};

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

VkDescriptorSet windowHandler::createDescriptorSet(VkDescriptorSetAllocateInfo const& CreateInfo, VkDescriptorBufferInfo const& ConfigInfo)
{

		return VkDescriptorSet();
}

// *****************************************************************************
