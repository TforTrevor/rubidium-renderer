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
} pushConstants;

layout(set = 0, binding = 0) uniform CameraBuffer{   
	mat4 view;
	mat4 projection;
} cameraData;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 color;

layout(location = 0) out vec3 fragColor;

void main() 
{
	fragColor = normal;

    //gl_Position = mvp.proj * mvp.view * mvp.model * vec4(position, 0.0, 1.0);
	gl_Position = cameraData.projection * cameraData.view * pushConstants.modelMatrix * vec4(position, 1.0);
}