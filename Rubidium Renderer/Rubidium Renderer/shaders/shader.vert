#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(binding = 0) uniform MVP {
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//} mvp;

layout( push_constant ) uniform Constants
{
	mat4 modelMatrix;
	vec4 objectPosition;
} pushConstants;

layout(set = 0, binding = 0) uniform CameraBuffer {
	mat4 view;
	mat4 projection;
	vec4 position;
} cameraData;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 color;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outWorldPos;

void main() 
{
	outColor = color;
	outWorldPos = vec3(pushConstants.modelMatrix * vec4(position, 1.0));
    outNormal = mat3(pushConstants.modelMatrix) * normal;

	gl_Position = cameraData.projection * cameraData.view * pushConstants.modelMatrix * vec4(position, 1.0);
}