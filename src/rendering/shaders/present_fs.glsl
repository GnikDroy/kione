#version 410 core

layout (location = 0) out vec4 FragColor;

in vec2 v_uv;

uniform sampler2D scene_texture;
uniform sampler2D bloom_texture;
uniform float bloom_intensity;

void main()
{
    vec3 color = texture(scene_texture, v_uv).rgb;
    color += texture(bloom_texture, v_uv).rgb * bloom_intensity;

    float peak = max(color.r, max(color.g, color.b));
    if (peak > 1.0)
    {
        float over = peak - 1.0;
        float compressed = 1.0 + over / (1.0 + 0.6 * over);
        vec3 hue = color / peak;
        float whiten = 0.6 * smoothstep(1.0, 3.5, peak);
        color = mix(hue, vec3(1.0), whiten) * compressed;
    }

    FragColor = vec4(color, 1.0);
}
