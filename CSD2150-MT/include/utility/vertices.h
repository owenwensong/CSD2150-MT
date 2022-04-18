/*!*****************************************************************************
 * @file    vertices.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    08 MAR 2022
 * @brief   This file contains common declarations for vertices used.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef UTILITY_VERTICES_HELPER_HEADER
#define UTILITY_VERTICES_HELPER_HEADER

#include <glm/glm.hpp>

struct VTX_2D_UV    { glm::vec2 m_Pos; glm::vec2 m_Tex; };
struct VTX_2D_RGB   { glm::vec2 m_Pos; glm::vec3 m_Col; };
struct VTX_2D_RGBA  { glm::vec2 m_Pos; glm::vec4 m_Col; };
struct VTX_3D_UV    { glm::vec3 m_Pos; glm::vec2 m_Tex; };
struct VTX_3D_UV_NML_TAN
{
  glm::vec3 m_Pos;
  glm::vec2 m_Tex;
  glm::vec3 m_Nml;
  glm::vec3 m_Tan;
};
struct VTX_3D_RGB   { glm::vec3 m_Pos; glm::vec3 m_Col; };
struct VTX_3D_RGBA  { glm::vec3 m_Pos; glm::vec4 m_Col; };
// todo: add VTX_XD_UV_XXXX

#endif//UTILITY_VERTICES_HELPER_HEADER
