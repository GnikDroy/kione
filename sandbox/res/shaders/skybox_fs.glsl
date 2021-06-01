#version 450 core

out vec4 FragColor;
in vec3 tex_coords;

uniform samplerCube cube_map;

void main()
{
    FragColor = texture(cube_map, vec3(-tex_coords.x, -tex_coords.y, tex_coords.z));
}