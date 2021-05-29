#version 450 core
out vec4 FragColor;

in vec4 color;
in vec2 tex_coord;
in vec3 norm;
in vec3 frag_pos;

uniform sampler2D tex;

uniform vec3 light_position;
uniform vec3 light_color;
uniform float ambient_strength;
uniform vec3 viewer_position;

void main()
{
    //calculate ambient light
    vec3 ambient_light = ambient_strength * light_color;

    //calculate diffuse light
    vec3 norm_norm = normalize(norm); // actually unnecessary, since the vector is already normalized.
    vec3 light_direction = normalize(light_position - frag_pos);
    float diffuse_strength = max(dot(norm_norm, light_direction), 0.0);
    vec3 diffuse_light = diffuse_strength * light_color;

    //calculate specular light
    float specular_strength = 0.9f;
    vec3 reflect_direction = normalize(reflect(-light_direction, norm_norm));
    vec3 viewer_direction = normalize(viewer_position - frag_pos);

    float specular_intensity = pow(max(dot(viewer_direction, reflect_direction), 0.0), 32);
    vec3 specular_light = specular_strength * specular_intensity * light_color;

    // set total light
    vec3 total_light = ambient_light + diffuse_light + specular_light;
    FragColor = texture(tex, tex_coord) * vec4(light_color, 1.0f) * vec4(total_light, 1.0f);
}