#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) in vec3 fragment_colour;
layout(location = 1) in vec2 fragment_texture_coordinate;

layout(location = 0) out vec4 out_colour;

void main()
{
    // out_colour = vec4(fragment_colour, 1.0);
    // out_colour = vec4(fragment_texture_coordinate, 0.0, 1.0);
    out_colour = texture(texture_sampler, fragment_texture_coordinate);
}
