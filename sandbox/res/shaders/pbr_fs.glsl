#version 450 core
out vec4 FragColor;

in vec3 color;
in vec2 texture_coordinate;

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

uniform Material material;

void main()
{
    FragColor = texture(material.albedo, texture_coordinate);
    // FragColor = vec4(color, 1.0f);
}