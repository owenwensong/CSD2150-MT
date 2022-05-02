#version 450

layout (set = 1, binding = 0) uniform u0f
{
  float u_AmbientStrength;
};

layout (set = 1, binding = 1) uniform u1v3
{
  vec3 u_LocalCamPos;
};

layout (set = 1, binding = 2) uniform u2v3
{
  vec3 u_LocalLightPos;
  vec3 u_LocalLightCol;
};

layout (set = 1, binding = 3) uniform sampler2D u_sColor;
layout (set = 1, binding = 4) uniform sampler2D u_sAmbient;
layout (set = 1, binding = 5) uniform sampler2D u_sNormal;
layout (set = 1, binding = 6) uniform sampler2D u_sRoughness;

layout(location = 0) in vec3 v_Pos;
layout(location = 1) in vec2 v_UV;
layout(location = 2) in mat3 v_TBN;

layout(location = 0) out vec4 f_FragColor;

layout(push_constant) uniform f_constants
{
  layout(offset = 64) float pc_Gamma;
};

//vec3 getNormal()// get from BC5 normal map
//{
//  vec2 normXY = texture(u_sNormal, v_UV).rg * 2.0 - 1.0;
//  return v_TBN * vec3(normXY, sqrt(1.0 - dot(normXY, normXY)));
//}

vec3 getNormal()// get from R8G8B8A8_UNORM for directx
{
  vec3 norm = texture(u_sNormal, v_UV).rgb * 2.0 - 1.0;
  norm.g = -norm.g;
  return v_TBN * norm;
}

void main()
{
  vec4 albedoColor = texture(u_sColor, v_UV);
	vec2 roughness = texture(u_sRoughness, v_UV).rg;// height mixed in g I think

	vec3 normal = getNormal();

	v_Pos + v_TBN[2] * roughness.g;// height? makes that cut on the skull kinda shine
	vec3 lightDir = normalize(u_LocalLightPos.xyz - v_Pos);
	float angle = max( 0, dot( normal, lightDir ));
	vec3 camDir = normalize( v_Pos - u_LocalLightPos.xyz );

	float specularAmt  = pow( max( 0, dot(normal, normalize( lightDir - camDir ))), mix( 1, 100, 1 - roughness.r ) );

	f_FragColor = albedoColor;
	f_FragColor.rgb *= u_AmbientStrength * texture(u_sAmbient, v_UV).rgb;
	f_FragColor.rgb += u_LocalLightCol * ( specularAmt * roughness.r + angle * albedoColor.rgb );
	f_FragColor.rgb = pow(f_FragColor.rgb, vec3(pc_Gamma));
}
