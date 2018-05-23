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
    mat4 normal_matrix;

} ubo;

//out vars same order as input vars
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_diffuse_colour;



void main()
{
    gl_Position = vec4(in_position, 1.0);
    // gl_Position = ubo.view * ubo.model * vec4(in_position, 1.0);
    // gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);

    //move mormal to world space
    // out_normal = normalize(mat3(ubo.model) * in_normal);
    out_normal = normalize(transpose( inverse(mat3(ubo.model)) ) * in_normal);

    out_uv = in_uv;
    out_diffuse_colour = in_diffuse_colour;
}
