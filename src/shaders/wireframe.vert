#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

//in vars
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 3) in vec3 in_colour;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

//out vars
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 out_colour;

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
    out_colour = in_colour;
}
