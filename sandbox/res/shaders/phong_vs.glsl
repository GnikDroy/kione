#version 450 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex_coord;

out vec2 texture_coordinate;
out vec3 normal;
out vec3 fragment_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);
    fragment_position = vec3(model * vec4(a_pos, 1.0));

    texture_coordinate = a_tex_coord;
    normal = mat3(transpose(inverse(model))) * a_norm;
}