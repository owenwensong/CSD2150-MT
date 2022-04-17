#version 450

layout (set = 1, binding = 0) uniform testy0
{
  vec3 Mul;
} uColorMul;

layout (set = 1, binding = 1) uniform sampler2D uTex2D;

layout(location = 0) in vec2 fragTex;
layout(location = 0) out vec4 outColor;

void main()
{
  outColor = texture(uTex2D, fragTex) * vec4(uColorMul.Mul, 1.0);
}
