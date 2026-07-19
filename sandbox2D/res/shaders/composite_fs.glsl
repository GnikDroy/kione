#version 410 core

layout (location = 0) out vec4 FragColor;

in vec2 v_uv;

uniform sampler2D albedo_texture;
uniform sampler2D light_texture;

void main()
{
    vec4 albedo = texture(albedo_texture, v_uv);
    vec3 light = texture(light_texture, v_uv).rgb;

    float peak = max(light.r, max(light.g, light.b));
    if (peak > 1.0)
    {
        float over = peak - 1.0;
        float compressed = 1.0 + over / (1.0 + 0.6 * over);
        vec3 hue = light / peak;
        float whiten = 0.6 * smoothstep(1.0, 3.5, peak);
        light = mix(hue, vec3(1.0), whiten) * compressed;
    }

    FragColor = vec4(albedo.rgb * light, albedo.a);
}
