/*!*****************************************************************************
 * @file    OBJLoader.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    19 MAR 2022
 * @brief   This file contains the definitions for an OBJ loader
 *          THIS FILE IS OBSOLETE AND NOT IN USE DUE TO THE INCLUSION OF ASSIMP.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <string>               // for individual lines from file
#include <charconv>             // string_view safe version of stoi/stof
#include <glm/glm.hpp>          // for generic vectors
#include <unordered_map>        // for map of vertexsig to vertex index
#include <utility/OBJLoader.h>  // declarations
#include <utility/CStrHash.hpp> // for token hashing to do token switch case

// *****************************************************************************
// **************************************************************** HELPERS ****

namespace MTU::Helper // helper namespace for MTU
{
  static constexpr const char WS[]{ " \t" };  // WhiteSpaces (no new line)
  static constexpr const char SS[]{ " \t/" }; // Slash Spaces (WS and /)

  struct OBJLine
  {
    size_t            m_FirstTokenHash{ 0 };
    std::string_view  m_Content       { /* Default empty */ };
  };

  struct OBJFaceContents { std::string_view m_Verts[3]; };

  struct OBJVertexSig
  {
    unsigned int m_PosIndex;
    unsigned int m_TexIndex;
    unsigned int m_NmlIndex;

    // equality operator required to compare keys in case of hash collision
    bool operator==(OBJVertexSig const& RHS) const { return m_PosIndex == RHS.m_PosIndex && m_TexIndex == RHS.m_TexIndex && m_NmlIndex == RHS.m_NmlIndex; }
  };

  struct OBJVertexSigHash // helper hasher
  {
    size_t operator()(OBJVertexSig const& toHash) const
    {
      return cstrHash(reinterpret_cast<const char*>(&toHash), sizeof(toHash));
    }
  };

  static OBJLine splitLine(std::string_view const& inStrView) noexcept
  {
    size_t FTS{ inStrView.find_first_not_of(WS) };    // First Token Start
    if (FTS == std::string_view::npos)return OBJLine{ /* Empty line */ };

    size_t FTE{ inStrView.find_first_of(WS, FTS) };   // First Token End
    if (FTE == std::string_view::npos)return OBJLine{ cstrHash(inStrView.data() + FTS, inStrView.length() - FTS) };
    
    size_t CS{ inStrView.find_first_not_of(WS, FTE) };// Content Start
    size_t CE{ inStrView.find_last_not_of(WS) + 1 };  // Content End
    return OBJLine
    {
      cstrHash(inStrView.data() + FTS, FTE - FTS),  // guaranteed valid alr
      CS == std::string_view::npos ? 
        std::string_view{ /* no content, create empty */ } :
        std::string_view{ inStrView.data() + CS, CE - CS }
    };
  }

  bool read3floats(glm::fvec3& outRef, std::string_view const& inContents) noexcept
  {
    size_t S0{ inContents.find_first_not_of(WS) };    // Token 0 Start
    if (S0 == std::string_view::npos)return false;    // Token 0 Not found
    size_t E0{ inContents.find_first_of(WS, S0) };    // Token 0 End

    size_t S1{ inContents.find_first_not_of(WS, E0) };// Token 1 Start
    if (S1 == std::string_view::npos)return false;    // Token 1 Not found
    size_t E1{ inContents.find_first_of(WS, S1) };    // Token 1 End

    size_t S2{ inContents.find_first_not_of(WS, E1) };// Token 2 Start
    if (S2 == std::string_view::npos)return false;    // Token 2 Not found
    size_t E2{ inContents.find_first_of(WS, S2) };    // Token 2 End

    if (E2 == std::string_view::npos)E2 = inContents.length();

    if (const char *pS0{ inContents.data() + S0 }, *pE0{ inContents.data() + E0 }; std::from_chars(pS0, pE0, outRef.x).ptr != pE0)return false;
    if (const char *pS1{ inContents.data() + S1 }, *pE1{ inContents.data() + E1 }; std::from_chars(pS1, pE1, outRef.y).ptr != pE1)return false;
    if (const char *pS2{ inContents.data() + S2 }, *pE2{ inContents.data() + E2 }; std::from_chars(pS2, pE2, outRef.z).ptr != pE2)return false;

    return true;
  }

  bool read2floats(glm::fvec2& outRef, std::string_view const& inContents) noexcept
  {
    size_t S0{ inContents.find_first_not_of(WS) };    // Token 0 Start
    if (S0 == std::string_view::npos)return false;    // Token 0 Not found
    size_t E0{ inContents.find_first_of(WS, S0) };    // Token 0 End

    size_t S1{ inContents.find_first_not_of(WS, E0) };// Token 1 Start
    if (S1 == std::string_view::npos)return false;    // Token 1 Not found
    size_t E1{ inContents.find_first_of(WS, S1) };    // Token 1 End

    if (E1 == std::string_view::npos)E1 = inContents.length();

    if (const char* pS0{ inContents.data() + S0 }, * pE0{ inContents.data() + E0 }; std::from_chars(pS0, pE0, outRef.x).ptr != pE0)return false;
    if (const char* pS1{ inContents.data() + S1 }, * pE1{ inContents.data() + E1 }; std::from_chars(pS1, pE1, outRef.y).ptr != pE1)return false;

    return true;
  }

  bool splitContents(OBJFaceContents& outRef, std::string_view const& inContents) noexcept
  {
    size_t S0{ inContents.find_first_not_of(WS) };    // Token 0 Start
    if (S0 == std::string_view::npos)return false;    // Token 0 Not found
    size_t E0{ inContents.find_first_of(WS, S0) };    // Token 0 End

    size_t S1{ inContents.find_first_not_of(WS, E0) };// Token 1 Start
    if (S1 == std::string_view::npos)return false;    // Token 1 Not found
    size_t E1{ inContents.find_first_of(WS, S1) };    // Token 1 End

    size_t S2{ inContents.find_first_not_of(WS, E1) };// Token 2 Start
    if (S2 == std::string_view::npos)return false;    // Token 2 Not found
    size_t E2{ inContents.find_first_of(WS, S2) };    // Token 2 End

    if (E2 == std::string_view::npos)E2 = inContents.length();

    outRef.m_Verts[0] = std::string_view{ inContents.data() + S0, E0 - S0 };
    outRef.m_Verts[1] = std::string_view{ inContents.data() + S1, E1 - S1 };
    outRef.m_Verts[2] = std::string_view{ inContents.data() + S2, E2 - S2 };
    return true;
  }

  bool getVertexSig(OBJVertexSig& outRef, std::string_view const& inContents) noexcept
  {
    outRef.m_PosIndex = outRef.m_TexIndex = outRef.m_NmlIndex = 0;

    size_t S0{ inContents.find_first_not_of(SS) };    // Token 0 Start
    if (S0 == std::string_view::npos)return false;    // Token 0 Not found
    size_t E0{ inContents.find_first_of(SS, S0) };    // Token 0 End

    if (E0 == std::string_view::npos)E0 = inContents.length();

    if (const char* pS0{ inContents.data() + S0 }, * pE0{ inContents.data() + E0 }; std::from_chars(pS0, pE0, outRef.m_PosIndex).ptr != pE0)return false;

    size_t S1{ inContents.find_first_not_of(SS, E0) };// Token 1 Start
    if (S1 == std::string_view::npos)return true;     // Token 1 Not found
    size_t E1{ inContents.find_first_of(SS, S1) };    // Token 1 End

    if (E1 == std::string_view::npos)E1 = inContents.length();

    if (const char* pS1{ inContents.data() + S1 }, * pE1{ inContents.data() + E1 }; std::from_chars(pS1, pE1, outRef.m_TexIndex).ptr != pE1)outRef.m_TexIndex = 0;

    size_t S2{ inContents.find_first_not_of(SS, E1) };// Token 2 Start
    if (S2 == std::string_view::npos)return true;     // Token 2 Not found
    size_t E2{ inContents.find_first_of(SS, S2) };    // Token 2 End

    if (E2 == std::string_view::npos)E2 = inContents.length();
    
    if (const char* pS2{ inContents.data() + S2 }, * pE2{ inContents.data() + E2 }; std::from_chars(pS2, pE2, outRef.m_NmlIndex).ptr != pE2)outRef.m_NmlIndex = 0;
  
    return true;
  }

}

// *****************************************************************************
// ******************************************************* PUBLIC FUNCTIONS ****

bool MTU::loadOBJ(std::ifstream& ifs, OBJOutputs& outputs, OBJLoadSettings settings)
{
  if (false == ifs.is_open())return false;// what are you doing with a bad ifs?
  
  std::vector<glm::vec3> OBJPositions;
  std::vector<glm::vec3> OBJNormals;
  std::vector<glm::vec2> OBJTexCoords;
  std::unordered_map<MTU::Helper::OBJVertexSig, uint16_t, MTU::Helper::OBJVertexSigHash> ExistingVertices;
  uint16_t nextIdx{ 0 };// next index when adding new vertex

  for (std::string lineStr; std::getline(ifs, lineStr);)
  {
    MTU::Helper::OBJLine lineOBJData{ MTU::Helper::splitLine(lineStr) };
    switch (lineOBJData.m_FirstTokenHash)
    {
    case "mtllib"_literalHash:
      // OBJ material
      outputs.m_Material = lineOBJData.m_Content;
      break;
    case "o"_literalHash:
    case "g"_literalHash:
      // OBJ name
      outputs.m_Name = lineOBJData.m_Content;
      break;
    case "v"_literalHash:
      // Vertex position
      //if (false == settings.m_bLoadPositions)break;
      if (glm::fvec3 tmpRes; MTU::Helper::read3floats(tmpRes, lineOBJData.m_Content))
      {
        OBJPositions.emplace_back(std::move(tmpRes));
      }
      else
      {
        return false;
      }
      break;
    case "vn"_literalHash:
      // Vertex normal
      if (false == settings.m_bLoadNormals)break;
      if (glm::fvec3 tmpRes; MTU::Helper::read3floats(tmpRes, lineOBJData.m_Content))
      {
        OBJNormals.emplace_back(std::move(tmpRes));
      }
      else
      {
        return false;
      }
      break;
    case "vt"_literalHash:
      // Vertex UV
      if (false == settings.m_bLoadTexCoords)break;
      if (glm::fvec2 tmpRes; MTU::Helper::read2floats(tmpRes, lineOBJData.m_Content))
      {
        OBJTexCoords.emplace_back(std::move(tmpRes));
      }
      else
      {
        return false;
      }
      break;
    case "f"_literalHash:
      // Triangle faces
      //if (false == settings.m_bLoadTriangles)break;
      if (MTU::Helper::OBJFaceContents OBJFace; MTU::Helper::splitContents(OBJFace, lineOBJData.m_Content))
      {
        for (size_t i{ 0 }; i < 4; ++i)// confirmed 3 valid vertices
        {
          if (MTU::Helper::OBJVertexSig tmpRes; MTU::Helper::getVertexSig(tmpRes, OBJFace.m_Verts[i]))
          {
            if (false == settings.m_bLoadNormals)tmpRes.m_NmlIndex = 0;
            if (false == settings.m_bLoadTexCoords)tmpRes.m_TexIndex = 0;
            if (decltype(ExistingVertices)::iterator found{ ExistingVertices.find(tmpRes) }; found == ExistingVertices.end())
            {
              // does not already exist
              outputs.m_Positions.emplace_back(OBJPositions[tmpRes.m_PosIndex - 1]);
              if (tmpRes.m_TexIndex != 0)
              {
                outputs.m_TexCoords.emplace_back(OBJPositions[tmpRes.m_TexIndex - 1]);
              }
              if (tmpRes.m_NmlIndex != 0)
              {
                outputs.m_Normals.emplace_back(OBJPositions[tmpRes.m_NmlIndex - 1]);
              }
              // not generating normals, maybe next time outside
              outputs.m_Triangles.emplace_back(nextIdx);
              ExistingVertices.emplace(std::move(tmpRes), nextIdx++);
            }
            else
            {
              // already exists
              outputs.m_Triangles.emplace_back(found->second);
            }
          }
        }
      }
      break;// will skip face if no 3 vertices. Ignore possiblity of 4 vertices
    default:
      break;
    }
  }
  return true;
}

// *****************************************************************************
