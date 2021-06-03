#version 450 core
out vec4 FragColor;

in vec2 texture_coordinate;
in vec3 fragment_position_tangent;
in mat3 TBN;
in vec3 light_position_tangent;
in vec3 viewer_position_tangent;

struct Material {
    sampler2D diffuse_0;
    sampler2D specular_0;
    sampler2D normal_0;
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

    // obtain normal in range [0,1]
    vec3 normal = texture(material.normal_0, texture_coordinate).rgb;
    // transform normal to range [-1,1]
    vec3 normal_norm = normalize(normal * 2.0 - 1.0);

    //calculate ambient light
    vec3 ambient_light = vec3(texture(material.diffuse_0, texture_coordinate)) * light.ambient;

    //calculate diffuse light
//    vec3 normal_norm = normalize(normal);
    vec3 light_direction = normalize(light_position_tangent - fragment_position_tangent);
    float diffuse_component = max(dot(normal_norm, light_direction), 0.0);
    vec3 diffuse_light = diffuse_component * vec3(texture(material.diffuse_0, texture_coordinate)) * light.diffuse;

    //calculate specular light
    vec3 reflect_direction = normalize(reflect(-light_direction, normal_norm));
    vec3 viewer_direction = normalize(viewer_position_tangent - fragment_position_tangent);

    float specular_component = pow(max(dot(viewer_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular_light = specular_component * vec3(texture(material.specular_0, texture_coordinate)) * light.specular;

    // set total light
    vec3 total_light = ambient_light + diffuse_light + specular_light;
    FragColor = vec4(total_light, 1.0f);
}