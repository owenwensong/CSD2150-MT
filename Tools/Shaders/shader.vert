#version 450

layout (set = 0, binding = 0) uniform testy0
{
  float offset;
} u0;

layout (set = 0, binding = 1) uniform testy1
{
  float offset;
} u1;

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 vertTex;

layout(location = 0) out vec2 fragTex;

layout(push_constant) uniform constants
{
  mat4 xform;
} PushConstants;

void main()
{
  gl_Position = PushConstants.xform * vec4(vertPos.x + u0.offset, vertPos.y + u1.offset, vertPos.z , 1.0);
  fragTex = vertTex;
}
