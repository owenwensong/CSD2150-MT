/*!*****************************************************************************
 * @file    graphicsHandler_vulkan.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    12 FEB 2022
 * @brief   This is the implementation for the graphicsHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <handlers/graphicsHandler_vulkan.h>
#include <vulkanHelpers/printWarnings.h>
#include <vulkanHelpers/vulkanDevice.h>			// already included in graphicsHandler

#include <utility/windowsInclude.h>
#include <vulkan/vulkan_win32.h>    // for surface extension name
#include <iostream>									// for std::cerr
#include <vector>
#include <array>
#include <span>

// *****************************************************************************
// **************************************************** NON-CLASS FUNCTIONS ****

/// @brief returns a vector of validation layer names. This function mostly 
///	       originates from xGPU as I have no idea what these layers do or why 
///	       you need just the 1 validation or the 8.
///        Instead of using std::move on a return value, this function 
///        assumes NRVO will be applied by the compiler.
/// @return validation layers to initialize the Vulkan instance
std::vector<const char*> getValidationLayers()
{
		constexpr static auto s_ValidationLayerNames_Alt1{ std::array{
			"VK_LAYER_KHRONOS_validation"
		} };
		constexpr static auto s_ValidationLayerNames_Alt2{ std::array{
			"VK_LAYER_GOOGLE_threading",     "VK_LAYER_LUNARG_parameter_validation",
			"VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_object_tracker",
			"VK_LAYER_LUNARG_image",         "VK_LAYER_LUNARG_core_validation",
			"VK_LAYER_LUNARG_swapchain",     "VK_LAYER_GOOGLE_unique_objects",
		} };

		uint32_t ValidationLayerCount{ 0 };
		if (VkResult tmpResult{ vkEnumerateInstanceLayerProperties(&ValidationLayerCount, nullptr) }; tmpResult != VK_SUCCESS || ValidationLayerCount <= 0)
		{
				printVKWarning(tmpResult, "Failed to get validation layer count"sv, true);
				return {};// return an empty vector, should this even be allowed to continue without validation layers?
		}
		auto LayerProperties{ std::make_unique<VkLayerProperties[]>(ValidationLayerCount) };
		if (VkResult tmpResult{ vkEnumerateInstanceLayerProperties(&ValidationLayerCount, LayerProperties.get()) }; tmpResult != VK_SUCCESS)
		{
				printVKWarning(tmpResult, "Failed to get validation layers"sv, true);
				return {};// return an empty vector, should this even be allowed to continue without validation layers?
		}

		std::vector<const char*> retval;// empty rn
		auto FilterValidationLayers
		{
			+[](std::vector<const char*>& Layers, std::span<VkLayerProperties> EnumeratedView, std::span<const char* const> FilterView)
			{
				Layers.clear();
				for (const auto& LayerEntry : EnumeratedView)
				{
					std::string LayerEntryName{ LayerEntry.layerName };
					for (const char* FilterEntry : FilterView)
					{// strings have internal hashes, so comparing a fixed string with a cString using the STL's operator== should be faster
						//std::cout << "LayerEntryName: " << LayerEntryName << std::endl;
						//std::cout << "FilterEntry: " << FilterEntry << std::endl;
						if (LayerEntryName == FilterEntry)
						{
							Layers.emplace_back(FilterEntry);
							break;
						}
						//std::cout << "nope!" << std::endl;
						// commented lines used as sanity check.
					}
				}
			}
		};
		FilterValidationLayers
		(
				retval,
				std::span<VkLayerProperties>{ LayerProperties.get(), ValidationLayerCount },
				std::span<const char* const>{ s_ValidationLayerNames_Alt1 }
		);
		//std::cout << "size: " << retval.size() << std::endl;
		if (retval.size() == s_ValidationLayerNames_Alt1.size())return retval;// NRVO return

		// control reaches here means size mismatch, try matching again
		printWarning("Failed to get the standard validation layers"sv);

		FilterValidationLayers
		(
				retval,
				std::span<VkLayerProperties>{ LayerProperties.get(), ValidationLayerCount },
				std::span<const char* const>{ s_ValidationLayerNames_Alt2 }
		);
		//std::cout << "size: " << retval.size() << std::endl;
		if (retval.size() == s_ValidationLayerNames_Alt2.size())return retval;// NRVO return

		// control reaches here means size mismatch again, return empty
		printWarning("Failed to get all the basic validation layers that we wanted"sv);

		return {};
}

// *****************************************************************************
// ************************************************* GRAPHICS HANDLER CLASS ****

graphicsHandler::graphicsHandler(bool enableDebugLayers, bool enableRenderDoc) :
		m_VkHandle{ createVkInstance(enableDebugLayers, enableRenderDoc) },
		m_VKDevice{ m_VkHandle, enableDebugLayers },
		bValidation{ enableDebugLayers ? 1 : 0 }
{
		if (bValidation)
		{
				std::cout << "graphicsHandler instance created! \nVkInstance status: "sv
									<< (m_VkHandle == VK_NULL_HANDLE ? "BAD"sv : "OK"sv)
									<< "\nvulkanDevice status: "sv
									<< (m_VKDevice.OK() ? "OK"sv : "BAD"sv)
									<< std::endl;
		}
}

graphicsHandler::~graphicsHandler()
{
		if (bValidation)
		{
				std::cout << "graphicsHandler instance destruct!"sv << std::endl;
		}
		vkDestroyInstance(m_VkHandle, nullptr);
}

bool graphicsHandler::VkInstanceOK() const noexcept
{
		return m_VkHandle != VK_NULL_HANDLE;
}

bool graphicsHandler::isDebugValidationOn() const noexcept
{
		return bValidation ? true : false;
}

// *************************************************************************
// ***************************************************** STATIC HELPERS ****

VkInstance graphicsHandler::createVkInstance(bool enableDebugLayers, bool enableRenderDoc)
{
		VkApplicationInfo vkAppInfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = "CSD2150MT",
			.applicationVersion = 1,
			.pEngineName = "CSD2150MT",
			.engineVersion = 1,
			.apiVersion = graphicsHandler::apiVersion
		};

		std::vector<const char*> Extensions
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		};

		std::vector<const char*> Layers;
		if (enableDebugLayers)
		{
				Layers = getValidationLayers();	// move construct
				if (Layers.size())
				{
						Extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
						Extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				}
		}

		if (enableRenderDoc)
		{
				Layers.emplace_back("VK_LAYER_RENDERDOC_Capture");
		}

		VkInstanceCreateInfo vkCreateInstanceInfo
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pApplicationInfo = &vkAppInfo,
			.enabledLayerCount = static_cast<decltype(VkInstanceCreateInfo::enabledLayerCount)>(Layers.size()),
			.ppEnabledLayerNames = Layers.size() ? Layers.data() : nullptr,
			.enabledExtensionCount = static_cast<decltype(VkInstanceCreateInfo::enabledExtensionCount)>(Extensions.size()),
			.ppEnabledExtensionNames = Extensions.size() ? Extensions.data() : nullptr
		};

		VkInstance retval{ VK_NULL_HANDLE };
		if (VkResult tmpResult{ vkCreateInstance(&vkCreateInstanceInfo, nullptr, &retval) }; tmpResult != VK_SUCCESS)
		{
				printVKWarning(tmpResult, "Failed to create the Vulkan Instance"sv, true);
		}
		return retval;
}

// *************************************************************************

// *****************************************************************************
