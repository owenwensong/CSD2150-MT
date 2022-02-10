/*!*****************************************************************************
 * @file    windowsInclude.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    10 FEB 2022
 * @brief   This file wraps the windows inclusion.
 * 
 *          https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers
 *          reference for: "Faster Builds with Smaller Header Files"
 *
 * Copyright (C) 2021 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef WINDOWS_INCLUDE_WRAPPING_HEADER
#define WINDOWS_INCLUDE_WRAPPING_HEADER

#ifndef NOMINMAX
#define NOMINMAX
#endif//NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif//WIN32_LEAN_AND_MEAN
#ifndef NOCOMM
#define NOCOMM
#endif//NOCOMM

#include <windows.h>

#endif//WINDOWS_INCLUDE_WRAPPING_HEADER
