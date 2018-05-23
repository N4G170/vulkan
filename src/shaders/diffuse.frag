#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;

//same order as out from vert shader
layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_diffuse_colour;

layout(location = 3) in vec4 in_light_direction;
layout(location = 4) in float in_diffuse_intensity;
layout(location = 5) in vec4 in_light_colour;
layout(location = 6) in float in_ambient_intensity;

//out vars
layout(location = 0) out vec4 out_colour;

void main()
{
	vec3 normal = in_normal;
	vec4 light_direction = in_light_direction;
	// vec3 normal = normalize(in_normal);//normalized in vert
	// vec4 light_direction = normalize(in_light_direction);//normalized in vert

	vec4 ambient = vec4(in_light_colour.rgb * in_ambient_intensity, 1.f);

	float diffuse_factor = max( dot(normal, light_direction.xyz), 0.f );
	vec4 diffuse = vec4(in_light_colour.rgb * in_diffuse_intensity * diffuse_factor, 1.f );

	vec3 colour = (ambient + diffuse).rgb * in_diffuse_colour;

	out_colour = vec4( colour * texture(texture_sampler, in_uv).rgb , 1.0);
}
