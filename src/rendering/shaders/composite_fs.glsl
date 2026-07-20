#version 410 core

layout (location = 0) out vec4 FragColor;

in vec2 v_uv;

uniform sampler2D albedo_texture;
uniform sampler2D light_texture;

void main()
{
    vec4 albedo = texture(albedo_texture, v_uv);
    vec3 light = texture(light_texture, v_uv).rgb;
    FragColor = vec4(albedo.rgb * light, albedo.a);
}
