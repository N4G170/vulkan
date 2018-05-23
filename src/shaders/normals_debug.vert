#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

//in vars from model vectos
layout(location = 0) in vec3 in_position;//vectex position in model space
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_diffuse_colour;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;

    vec4 camera_position;

    // bool apply_light;
    vec4 light_position;
    vec4 light_colour;
    float diffuse_intensity;
    float ambient_intensity;
} ubo;

//out vars same order as input vars
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_diffuse_colour;

layout(location = 3) out vec4 out_light_direction;
layout(location = 4) out float out_diffuse_intensity;
layout(location = 5) out vec4 out_light_colour;
layout(location = 6) out float out_ambient_intensity;


void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);

    // vec4 pos = ubo.view * ubo.model * vec4(in_position, 1.0);

    //light direction
    vec4 world_position = ubo.model * vec4(in_position, 0.f);

    //send light
    out_light_direction = ubo.light_position - world_position;
    out_diffuse_intensity = ubo.diffuse_intensity;
    out_light_colour = ubo.light_colour;
    out_ambient_intensity = ubo.ambient_intensity;

    //move mormal to world space
    // out_normal = pos.xyz * in_normal;
    out_normal = (ubo.model * vec4(in_normal, 0.f)).rgb;
    out_uv = in_uv;
    out_diffuse_colour = in_diffuse_colour;
}
