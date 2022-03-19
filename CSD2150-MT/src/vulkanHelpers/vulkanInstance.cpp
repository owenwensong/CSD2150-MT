/*!*****************************************************************************
 * @file    vulkanInstance.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    14 FEB 2022
 * @brief   This is the implementation for the vulkan instance helper class
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/vulkanInstance.h>
#include <vulkanHelpers/printWarnings.h>
#include <utility/windowsInclude.h>
#include <vulkan/vulkan_win32.h>    // for surface extension name
#include <iostream>	// for debugCallback cerr
#include <memory>
#include <array>
#include <span>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageType, 
		VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, 
		void* pUserData
)
{		// >= rather than & to cover VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT as well
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
				std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		}

		return VK_FALSE;
}

// *****************************************************************************
// ************************************************************** CTOR/DTOR ****

vulkanInstance::vulkanInstance(bool enableDebugLayers, bool enableRenderDoc) :
		m_VkHandle{ createVkInstance(enableDebugLayers, enableRenderDoc) },
		m_pVKAllocator{ nullptr },
		m_DebugMessenger{ VK_NULL_HANDLE },
		bValidation{ enableDebugLayers ? 1 : 0 },
		bRenderDoc{ enableRenderDoc ? 1 : 0 }
{
		if (OK() && bValidation)
		{
				VkDebugUtilsMessengerCreateInfoEXT CreateInfo
				{
						.sType					{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT },
						.messageSeverity{ VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT },
						.messageType		{ VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT },
						.pfnUserCallback{ debugCallback }
				};
				PFN_vkCreateDebugUtilsMessengerEXT pFnCreateDebugUtilsMessenger
				{
						reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
						(
								vkGetInstanceProcAddr(m_VkHandle, "vkCreateDebugUtilsMessengerEXT")
						)
				};
				if (pFnCreateDebugUtilsMessenger == nullptr)
				{
						printWarning("FAILED TO GET PFN_vkCreateDebugUtilsMessengerEXT"sv, true);
				}
				else if (VkResult tmpRes{ pFnCreateDebugUtilsMessenger(m_VkHandle, &CreateInfo, m_pVKAllocator, &m_DebugMessenger) }; tmpRes != VK_SUCCESS)
				{
						printVKWarning(tmpRes, "FAILED TO SET UP DEBUG MESSENGER"sv, true);
				}
		}
}

vulkanInstance::~vulkanInstance()
{
		if (m_DebugMessenger)
		{
				if (PFN_vkDestroyDebugUtilsMessengerEXT pFnDestroyDebugUtilsMessenger{ reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_VkHandle, "vkDestroyDebugUtilsMessengerEXT")) }; pFnDestroyDebugUtilsMessenger != nullptr)
				{
						pFnDestroyDebugUtilsMessenger(m_VkHandle, m_DebugMessenger, m_pVKAllocator);
				}
				else
				{
						printWarning("FAILED TO GET PFN_vkDestroyDebugUtilsMessengerEXT"sv, true);
				}

		}
		vkDestroyInstance(m_VkHandle, nullptr);
}

// *****************************************************************************

VkInstance vulkanInstance::createVkInstance(bool enableDebugLayers, bool enableRenderDoc)
{
		VkApplicationInfo vkAppInfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = "CSD2150MT",
			.applicationVersion = 1,
			.pEngineName = "CSD2150MT",
			.engineVersion = 1,
			.apiVersion = vulkanInstance::apiVersion
		};

		std::vector<const char*> Extensions
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		};

		// only VK 1.0 needs maintenance1 to use negative viewport
		if (vkAppInfo.apiVersion == VK_VERSION_1_0)Extensions.emplace_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);

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

bool vulkanInstance::OK() const noexcept
{
		return m_VkHandle != VK_NULL_HANDLE;
}

bool vulkanInstance::isDebugValidationOn() const noexcept
{
		return bValidation ? true : false;
}

bool vulkanInstance::isRenderDocOn() const noexcept
{
		return bRenderDoc ? true : false;
}

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
