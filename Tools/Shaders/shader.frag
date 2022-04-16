#version 450

layout(set = 1, binding = 0) uniform testy0
{
  vec3 Mul;
} uColorMul;

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main()
{
  outColor = vec4(uColorMul.Mul * fragColor, 1.0);
}
