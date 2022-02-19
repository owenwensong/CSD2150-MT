#version 450

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[]
(
  vec2(-0.5,  0.5),
  vec2( 0.0, -0.5),
  vec2( 0.5,  0.5)
);

vec3 colors[3] = vec3[]
(
  vec3(1.0, 0.0, 0.0),
  vec3(0.0, 0.0, 1.0),
  vec3(0.0, 1.0, 0.0)
);

layout(push_constant) uniform constants
{
  float aspectRatio;
} PushConstants;

void main()
{
	vec2 vpos = positions[gl_VertexIndex];
	vpos.x *= PushConstants.aspectRatio;
    gl_Position = vec4(vpos, 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
