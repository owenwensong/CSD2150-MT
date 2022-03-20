/*!*****************************************************************************
 * @file    OBJLoader.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    19 MAR 2022
 * @brief   This file contains the declarations for an OBJ loader
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef UTILITY_OBJ_LOADER_HELPER_HEADER
#define UTILITY_OBJ_LOADER_HELPER_HEADER

#include <vector>   // for outputs
#include <fstream>  // for file source

namespace MTU
{
  struct OBJOutputs
  {
    std::string             m_Name;
    std::string             m_Material;
    std::vector<glm::vec3>  m_Positions;
    std::vector<uint16_t>   m_Triangles;
    std::vector<glm::vec3>  m_Normals;
    std::vector<glm::vec2>  m_TexCoords;
  };

  struct OBJLoadSettings
  {
    bool m_bLoadNormals   { false };
    bool m_bLoadTexCoords { true };
  };

  bool loadOBJ(std::ifstream& ifs, OBJOutputs& outputs, OBJLoadSettings = {});
}

#endif//UTILITY_OBJ_LOADER_HELPER_HEADER
