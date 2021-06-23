#version 450 core

layout (location = 0) out vec4 FragColor;

in vec4 color;
in vec2 texture_coordinate;
flat in uint texture_index;

uniform sampler2D texture_list[32];

void main()
{
    FragColor = color * texture(texture_list[texture_index], texture_coordinate);
}