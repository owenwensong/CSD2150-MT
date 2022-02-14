/*!*****************************************************************************
 * @file    printWarnings.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    14 FEB 2022
 * @brief   This contains declarations for functions to print warnings
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <vulkan/vulkan.h>
#include <source_location>  // for error/warning reporting
#include <string>           // for std::string_view

#pragma warning (disable: 26812)// seriously shut up about enums

using namespace std::string_view_literals;	// for literal operator sv

std::string_view VKErrorToString(VkResult errCode) noexcept;
void printVKWarning(VkResult errCode, std::string_view msg, bool isError = false, std::source_location const& errLoc = std::source_location::current()) noexcept;
void printWarning(std::string_view msg, bool isError = false, std::source_location const& errLoc = std::source_location::current()) noexcept;
