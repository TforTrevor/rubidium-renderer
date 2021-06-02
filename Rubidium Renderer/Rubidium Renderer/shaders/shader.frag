#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 albedo;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform SceneData {
	vec4 ambientColor;
	vec4 sunDirection;
	vec4 sunColor;
} sceneData;

void main() 
{
	vec3 color = albedo * sceneData.sunColor.rgb * dot(normal, sceneData.sunDirection.xyz);
	color += sceneData.ambientColor.rgb;
    outColor = vec4(color, 1.0);
}