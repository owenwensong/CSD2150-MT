/*!*****************************************************************************
 * @file    matrixTransforms.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    08 MAR 2022
 * @brief   This file contains the definitions to a few matrix transformation
 *          helper functions
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <utility/matrixTransforms.h>

// based on a previous semi-optimized implementation I made. the invMat output 
// may be useful if you need to bring something to model space for some reason.
// It was previously used to bring point lights to model space.
glm::mat3 MTU::axisAngleRotation(glm::vec3 rotAxis, float rotRad, glm::mat3* invMat) noexcept
{
  rotAxis = glm::normalize(rotAxis);
  float cosRot{ cosf(rotRad) }, sinRot{ sinf(rotRad) };
  glm::mat3 tempMa  // tensor product expression
  {
    rotAxis.x * rotAxis.x, rotAxis.x * rotAxis.y, rotAxis.x * rotAxis.z,
    rotAxis.x * rotAxis.y, rotAxis.y * rotAxis.y, rotAxis.y * rotAxis.z,
    rotAxis.x * rotAxis.z, rotAxis.y * rotAxis.z, rotAxis.z * rotAxis.z
  };
  glm::mat3 tempMb  // skew symmetric expression
  {
    0.0f, rotAxis.z, -rotAxis.y,
    -rotAxis.z, 0.0f, rotAxis.x,
    rotAxis.y, -rotAxis.x, 0.0f
  };
  tempMa = glm::mat3
  {
    cosRot, 0.0f, 0.0f,
    0.0f, cosRot, 0.0f,
    0.0f, 0.0f, cosRot
  } + ((1.0f - cosRot) * tempMa);
  tempMb *= sinRot;
  if (invMat)*invMat = tempMa - tempMb;
  return tempMa + tempMb;
}
