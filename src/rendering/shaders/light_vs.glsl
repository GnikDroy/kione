#version 410 core
layout (location = 0) in vec2 a_local;

out vec2 v_local;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    v_local = a_local;
    gl_Position = projection * view * model * vec4(a_local, 0.0, 1.0);
}
