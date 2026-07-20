#version 410 core

layout (location = 0) out vec4 FragColor;

in vec2 v_uv;

uniform sampler2D scene_texture;
uniform float threshold;

void main()
{
    vec3 color = texture(scene_texture, v_uv).rgb;
    float brightness = max(color.r, max(color.g, color.b));

    float knee = threshold * 0.5 + 1e-4;
    float soft = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee);
    float contribution = max(soft, brightness - threshold) / max(brightness, 1e-4);

    FragColor = vec4(color * max(contribution, 0.0), 1.0);
}
