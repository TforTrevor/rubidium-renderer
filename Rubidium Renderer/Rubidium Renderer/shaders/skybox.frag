#version 460 core

layout(location = 0) in vec3 inPos;

layout(set = 1, binding = 0) uniform samplerCube environmentMap;

layout(location = 0) out vec4 fragColor;

void main()
{		
    vec3 pos = vec3(inPos.x, inPos.y, inPos.z);
    vec3 envColor = texture(environmentMap, pos).rgb;
  
    fragColor = vec4(envColor, 1.0);
}