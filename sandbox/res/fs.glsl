#version 450 core
out vec4 FragColor;

in vec4 color;
in vec2 tex_coord;

uniform sampler2D tex;

void main()
{
    FragColor = texture(tex, tex_coord);
}