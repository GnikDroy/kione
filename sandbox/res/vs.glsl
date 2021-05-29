#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 acolor;
layout (location = 2) in vec2 atex_coord;

out vec4 color;
out vec2 tex_coord;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    tex_coord = atex_coord;
    color = acolor;
}