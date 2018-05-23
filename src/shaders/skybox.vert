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
} ubo;

//out vars same order as input vars
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 out_uv_dir;


void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);

    out_uv_dir = in_position;
    out_uv_dir.x *= -1;  
}
