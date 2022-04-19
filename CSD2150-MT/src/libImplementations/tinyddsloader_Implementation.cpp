/*!*****************************************************************************
 * @file    tinyddsloader_Implementation.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    30 MAR 2022
 * @brief   This file is where tinyddsloader is compiled.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef TINYDDS_IMPLEMENTED // guard in case enabling unity build makes
#define TINYDDS_IMPLEMENTED // implementation definition leak everywhere

#pragma warning (disable : 4244 26451 26495 26812)// disable library warnings

#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>
#undef TINYDDSLOADER_IMPLEMENTATION

#pragma warning (default : 4244 26451 26495 26812)// reenable warnings

#endif//TINYDDS_IMPLEMENTED
