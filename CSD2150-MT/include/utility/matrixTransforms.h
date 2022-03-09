/*!*****************************************************************************
 * @file    matrixTransforms.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    08 MAR 2022
 * @brief   This file contains the declaration to a few matrix transformation
 *          helper functions
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef UTILITY_MATRIX_TRANSFORMS_HELPER_HEADER
#define UTILITY_MATRIX_TRANSFORMS_HELPER_HEADER

#include <glm/glm.hpp>

namespace MTU // MT Utility
{
  glm::mat3 axisAngleRotation(glm::vec3 rotAxis, float rotRad, glm::mat3* invMat) noexcept;
}


#endif//UTILITY_MATRIX_TRANSFORMS_HELPER_HEADER
