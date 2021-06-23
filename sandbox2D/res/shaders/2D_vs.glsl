#version 450 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_tex_coord;
layout (location = 3) in uint a_tex_index;

out vec4 color;
out vec2 texture_coordinate;
flat out uint texture_index;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(a_pos, 1.0);

    texture_coordinate = a_tex_coord;
    texture_index = a_tex_index;
    color = a_color;
}