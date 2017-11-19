#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

//in vars
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec2 texture_coordinate;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} objectMatrix;

//out vars
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 fragment_colour;
layout(location = 1) out vec2 fragment_texture_coordinate;


void main()
{
    gl_Position = objectMatrix.projection * objectMatrix.view * objectMatrix.model * vec4(position, 0.0, 1.0);
    fragment_colour = colour;
    fragment_texture_coordinate = texture_coordinate;
}
