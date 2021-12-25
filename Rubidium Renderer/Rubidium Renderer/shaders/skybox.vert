#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inColor;

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 projection;
	mat4 view[6];
} cameraData;

layout(location = 0) out vec3 outPos;

void main()
{
    outPos = inPos;

    mat4 rotView = mat4(mat3(cameraData.view[0])); // remove translation from the view matrix
    vec4 clipPos = cameraData.projection * rotView * vec4(outPos, 1.0);

    gl_Position = clipPos.xyww;
}