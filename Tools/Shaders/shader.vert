#version 450

layout(binding = 0) uniform fixedUniform
{
  float offsetTest;
} FU;

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 vertTex;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform constants
{
  mat4 xform;
} PushConstants;

void main()
{
  gl_Position = PushConstants.xform * vec4(vertPos.x + FU.offsetTest, vertPos.y, vertPos.z , 1.0);
  fragColor = vec3(vertTex, 1.0);
}
