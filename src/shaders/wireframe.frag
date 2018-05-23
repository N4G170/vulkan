#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

layout(location = 0) in vec3 in_colour;
layout(location = 0) out vec4 out_colour;

void main()
{
    // out_colour = vec4(in_colour * 1.5, 1.0);
    out_colour = vec4(vec3(1) * 1.5, 1.0);
}
