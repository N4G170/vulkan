#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

layout(set = 1, binding = 0) uniform samplerCube texture_sampler;

//same order as out from vert shader
layout(location = 0) in vec3 in_uv_dir;

//out vars
layout(location = 0) out vec4 out_colour;

void main()
{
	out_colour = texture(texture_sampler, in_uv_dir);
}
