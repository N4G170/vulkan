#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;

//same order as out from vert shader
layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_diffuse_colour;
layout(location = 3) in vec3 in_specular_colour;
layout(location = 4) in vec3 in_view_direction;

layout(location = 5) in vec4 in_light_direction;
layout(location = 6) in float in_diffuse_intensity;
layout(location = 7) in vec4 in_light_colour;
layout(location = 8) in float in_ambient_intensity;
layout(location = 9) in float in_specular_intensity;

//out vars
layout(location = 0) out vec4 out_colour;

void main()
{
	vec3 normal = normalize(in_normal);
	// float light_distance = length(in_light_direction);
	vec4 light_direction = normalize(in_light_direction);
	vec3 view_direction = normalize(in_view_direction);


	vec4 ambient = vec4(in_light_colour.rgb * in_ambient_intensity, 1.f);

	float diffuse_factor = max( dot(normal, -light_direction.rgb), 0.f );
	vec4 diffuse = vec4(in_light_colour.rgb * in_diffuse_intensity * diffuse_factor, 1.f );

	vec4 specular = vec4(0.f);
	vec3 colour = vec3(0.f);
	if(diffuse_factor > 0)//fragment is affected by light
	{
		colour = vec3(1,0,0);
		vec3 reflection = normalize(reflect(normal, light_direction.xyz));
		float specular_factor = dot(reflection, view_direction);

		if(specular_factor > 0)
		{
			colour = vec3(1,1,0);
			specular_factor = pow(specular_factor, 32);
			// specular = vec4( in_specular_colour * in_specular_intensity * specular_factor, 1.f );
			specular = vec4( colour * in_specular_intensity * specular_factor, 1.f );
		}
	}

	// vec3 colour = (ambient + diffuse + specular).rgb * in_diffuse_colour;
	colour = (ambient + diffuse + specular).rgb * in_diffuse_colour;

	out_colour = vec4( colour * texture(texture_sampler, in_uv).rgb , 1.0);
	// vec3 c = in_diffuse_colour.rgb * normal_dot;
	// out_colour = vec4( in_diffuse_colour.rgb + c, 1.0);
}
