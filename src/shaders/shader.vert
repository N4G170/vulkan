#version 450
#extension GL_ARB_separate_shader_objects : enable//needed for vulkan

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 fragment_colour;

float wx = 0.1136;
float wy = 0.08;
float hx = 0.202;
float hy = 0.1428;
float ox = -0.0568;
float oy = -0.2439;

vec2 A = vec2(ox          , oy);
vec2 B = vec2(ox + wx     , oy);
vec2 C = vec2(ox + wx + wy, oy + hy);
vec2 D = vec2(ox + wx + wy, oy + hy + hx);
vec2 E = vec2(ox + wx     , oy + hy + hx + hy);
vec2 F = vec2(ox          , oy + hy + hx + hy);
vec2 G = vec2(ox - wy     , oy + hy + hx);
vec2 H = vec2(ox - wy     , oy + hy);

vec3 cA = vec3(1.0, 0.0, 0.0);
vec3 cB = vec3(1.0, 1.0, 0.0);
vec3 cC = vec3(1.0, 1.0, 1.0);
vec3 cD = vec3(1.0, 0.0, 1.0);
vec3 cE = vec3(0.0, 1.0, 0.0);
vec3 cF = vec3(0.0, 1.0, 1.0);
vec3 cG = vec3(0.0, 0.0, 1.0);
vec3 cH = vec3(1.0, 1.0, 1.0);

vec2 positions[18] = vec2[]
(
    A,B,C,
    A,C,D,
    A,D,E,
    A,E,F,
    A,F,G,
    A,G,H
);

vec3 colours[18] = vec3[]
(
    cA,cB,cC,
    cA,cC,cD,
    cA,cD,cE,
    cA,cE,cF,
    cA,cF,cG,
    cA,cG,cH
);

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragment_colour = colours[gl_VertexIndex];
}
