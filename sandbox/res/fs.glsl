#version 450 core
out vec4 FragColor;

in vec2 texture_coordinate;
in vec3 normal;
in vec3 fragment_position;

struct Material {
    sampler2D diffuse_0;
    sampler2D diffuse_1;
    sampler2D diffuse_2;
    sampler2D diffuse_3;

    sampler2D specular_0;
    sampler2D specular_1;
    sampler2D specular_2;
    sampler2D specular_3;

    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewer_position;
uniform Material material;
uniform Light light;

void main()
{
    //calculate ambient light
    vec3 ambient_light = vec3(texture(material.diffuse_0, texture_coordinate)) * light.ambient;

    //calculate diffuse light
    vec3 normal_norm = normalize(normal);
    vec3 light_direction = normalize(light.position - fragment_position);
    float diffuse_component = max(dot(normal_norm, light_direction), 0.0);
    vec3 diffuse_light = diffuse_component * vec3(texture(material.diffuse_0, texture_coordinate)) * light.diffuse;

    //calculate specular light
    vec3 reflect_direction = normalize(reflect(-light_direction, normal_norm));
    vec3 viewer_direction = normalize(viewer_position - fragment_position);

    float specular_component = pow(max(dot(viewer_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular_light = specular_component * vec3(texture(material.specular_0, texture_coordinate)) * light.specular;

    // set total light
    vec3 total_light = ambient_light + diffuse_light + specular_light;
    FragColor = vec4(total_light, 1.0f);
}