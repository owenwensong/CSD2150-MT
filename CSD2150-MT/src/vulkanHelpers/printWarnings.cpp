/*!*****************************************************************************
 * @file    printWarnings.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    14 FEB 2022
 * @brief   This contains the implementation for functions to print warnings
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkanHelpers/printWarnings.h>
#include <iostream>

std::string_view VKErrorToString(VkResult errCode) noexcept
{
		switch (errCode)
		{
#define STRINGIFYVKERRHELPER(A, B) A##B
#define STRINGIFYVKERR(code) STRINGIFYVKERRHELPER(case VK_ ##code: return #code, sv)
				STRINGIFYVKERR(NOT_READY);
				STRINGIFYVKERR(TIMEOUT);
				STRINGIFYVKERR(EVENT_SET);
				STRINGIFYVKERR(EVENT_RESET);
				STRINGIFYVKERR(INCOMPLETE);
				STRINGIFYVKERR(ERROR_OUT_OF_HOST_MEMORY);
				STRINGIFYVKERR(ERROR_OUT_OF_DEVICE_MEMORY);
				STRINGIFYVKERR(ERROR_INITIALIZATION_FAILED);
				STRINGIFYVKERR(ERROR_DEVICE_LOST);
				STRINGIFYVKERR(ERROR_MEMORY_MAP_FAILED);
				STRINGIFYVKERR(ERROR_LAYER_NOT_PRESENT);
				STRINGIFYVKERR(ERROR_EXTENSION_NOT_PRESENT);
				STRINGIFYVKERR(ERROR_FEATURE_NOT_PRESENT);
				STRINGIFYVKERR(ERROR_INCOMPATIBLE_DRIVER);
				STRINGIFYVKERR(ERROR_TOO_MANY_OBJECTS);
				STRINGIFYVKERR(ERROR_FORMAT_NOT_SUPPORTED);
				STRINGIFYVKERR(ERROR_FRAGMENTED_POOL);
				STRINGIFYVKERR(ERROR_UNKNOWN);
				STRINGIFYVKERR(ERROR_OUT_OF_POOL_MEMORY);
				STRINGIFYVKERR(ERROR_INVALID_EXTERNAL_HANDLE);
				STRINGIFYVKERR(ERROR_FRAGMENTATION);
				STRINGIFYVKERR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
				STRINGIFYVKERR(ERROR_SURFACE_LOST_KHR);
				STRINGIFYVKERR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
				STRINGIFYVKERR(SUBOPTIMAL_KHR);
				STRINGIFYVKERR(ERROR_OUT_OF_DATE_KHR);
				STRINGIFYVKERR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
				STRINGIFYVKERR(ERROR_VALIDATION_FAILED_EXT);
				STRINGIFYVKERR(ERROR_INVALID_SHADER_NV);
				STRINGIFYVKERR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
				STRINGIFYVKERR(ERROR_NOT_PERMITTED_EXT);
				STRINGIFYVKERR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
				STRINGIFYVKERR(ERROR_PIPELINE_COMPILE_REQUIRED_EXT);
#undef STRINGIFYVKERR
#undef STRINGIFYVKERRHELPER
		default: return "UNKNOWN_ERROR"sv;
		}
}

void printVKWarning(VkResult errCode, std::string_view msg, bool isError, std::source_location const& errLoc) noexcept
{
		std::cerr << errLoc.file_name()
							<< '('
							<< errLoc.line()
							<< '/'
							<< errLoc.column()
							<< ") ["sv
							<< errLoc.function_name()
							<< (isError ? "] ERROR VK"sv : "] WARNING VK"sv)
							<< static_cast<long long>(errCode)
							<< " ("sv
							<< VKErrorToString(errCode)
							<< "): "sv
							<< msg
							<< std::endl;
}

void printWarning(std::string_view msg, bool isError, std::source_location const& errLoc) noexcept
{
		std::cerr << errLoc.file_name()
							<< '('
							<< errLoc.line()
							<< '/'
							<< errLoc.column()
							<< ") ["sv
							<< errLoc.function_name()
							<< (isError ? "] ERROR: "sv : "] WARNING: "sv)
							<< msg
							<< std::endl;
}
