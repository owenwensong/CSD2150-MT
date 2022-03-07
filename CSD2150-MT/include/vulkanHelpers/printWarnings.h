/*!*****************************************************************************
 * @file    printWarnings.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    14 FEB 2022
 * @brief   This contains declarations for functions to print warnings
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef VULKAN_HELPERS_PRINT_WARNINGS_HEADER
#define VULKAN_HELPERS_PRINT_WARNINGS_HEADER

#include <vulkan/vulkan.h>
#include <source_location>  // for error/warning reporting
#include <string>           // for std::string_view

#pragma warning (disable: 26812)// seriously shut up about enums

//using std::string_view_literals::operator""sv;// produces warning C4455 due to compiler bug
using namespace std::string_view_literals;	// for literal operator sv

std::string_view VKErrorToString(VkResult errCode) noexcept;
void printVKWarning(VkResult errCode, std::string_view msg, bool isError = false, std::source_location const& errLoc = std::source_location::current()) noexcept;
void printWarning(std::string_view msg, bool isError = false, std::source_location const& errLoc = std::source_location::current()) noexcept;

#endif//VULKAN_HELPERS_PRINT_WARNINGS_HEADER
