#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;

//same order as out from vert shader
layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_diffuse_colour;


//out vars
layout(location = 0) out vec4 out_colour;

void main()
{
	// out_colour = vec4(texture(texture_sampler, in_uv).rgb * in_diffuse_colour , 1.0);
	out_colour = vec4(texture(texture_sampler, in_uv).rgb * in_diffuse_colour , texture(texture_sampler, in_uv).a);
}
