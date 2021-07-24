#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_debug_printf : enable

layout(set = 0, binding = 0) uniform CameraBuffer {
	mat4 view;
	mat4 projection;
	vec4 position;
} cameraData;

struct ObjectData {
	mat4 modelMatrix;
	vec4 albedo;
	vec4 maskMap;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} objectBuffer;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 color;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outWorldPos;
layout(location = 3) out vec4 outMaterialAlbedo;
layout(location = 4) out vec4 outMaterialMaskMap;

void main() 
{
	//debugPrintfEXT("%f ", objectBuffer.objects[gl_InstanceIndex].modelMatrix[3][0]);
	//debugPrintfEXT("%i ", gl_BaseInstance);
	outColor = color;
	outWorldPos = vec3(objectBuffer.objects[gl_InstanceIndex].modelMatrix * vec4(position, 1.0));
    outNormal = mat3(objectBuffer.objects[gl_InstanceIndex].modelMatrix) * normal;
	outMaterialAlbedo = objectBuffer.objects[gl_InstanceIndex].albedo;
	outMaterialMaskMap = objectBuffer.objects[gl_InstanceIndex].maskMap;
	//outMaterialAlbedo = vec4(1, 1, 1, 1);
	//outMaterialMaskMap = vec4(1, 1, 1, 1);

	gl_Position = cameraData.projection * cameraData.view * objectBuffer.objects[gl_InstanceIndex].modelMatrix * vec4(position, 1.0);
}