#version 460

layout(set = 0, binding = 0) uniform CameraBuffer {
	mat4 view;
	mat4 projection;
	vec4 position;
} cameraData;

struct ObjectData {
	mat4 modelMatrix;
	mat4 MVP;
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
layout(location = 3) out vec2 outTexCoord;

void main() 
{
	//debugPrintfEXT("%f ", objectBuffer.objects[gl_InstanceIndex].modelMatrix[3][0]);
	//debugPrintfEXT("%i ", gl_BaseInstance);
	outColor = color;
	outWorldPos = vec3(objectBuffer.objects[gl_BaseInstance].modelMatrix * vec4(position, 1.0));
    outNormal = mat3(objectBuffer.objects[gl_BaseInstance].modelMatrix) * normal;
	//outNormal = normal;
	outTexCoord = texCoord;
	//outMaterialAlbedo = vec4(1, 1, 1, 1);
	//outMaterialMaskMap = vec4(1, 1, 1, 1);

	gl_Position = objectBuffer.objects[gl_BaseInstance].MVP * vec4(position, 1.0);
}