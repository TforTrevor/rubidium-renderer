#version 450
#extension GL_ARB_separate_shader_objects : enable

/*layout(binding = 0) uniform Transform {
    mat4 model;
    mat4 view;
    mat4 proj;
} transform;*/

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

void main() 
{
	fragColor = color;

    //gl_Position = transform.proj * transform.view * transform.model * vec4(position, 0.0, 1.0);
	gl_Position = vec4(position, 0.0, 1.0);
}