#version 410 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;
layout (location = 2) in vec3 a_norm;
layout (location = 3) in vec2 a_texture_coordinate;

out vec3 color;
out vec2 texture_coordinate;

struct Material {
    sampler2D albedo;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D normal;
    sampler2D ambient_occlusion;
    
    int has_albedo;
    int has_metallic;
    int has_roughness;
    int has_normal;
    int has_ambient_occlusion;

    vec3 albedo_value;
    float metallic_value;
    float roughness_value;
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform Material material;

void main()
{
    gl_Position = projection * view * model * vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);
    color = a_color;
    texture_coordinate = a_texture_coordinate;
}