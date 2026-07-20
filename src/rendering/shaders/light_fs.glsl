#version 410 core

layout (location = 0) out vec4 FragColor;

in vec2 v_local;

uniform int mode; // 0 = point, 1 = spot, 2 = sprite
uniform vec3 color;
uniform float cos_inner;
uniform float cos_outer;
uniform sampler2D light_texture;

void main()
{
    if (mode == 2) {
        vec2 uv = v_local * 0.5 + 0.5;
        FragColor = vec4(color * texture(light_texture, uv).rgb, 1.0);
        return;
    }

    float dist = length(v_local);
    float atten = clamp(1.0 - dist, 0.0, 1.0);
    atten *= atten;

    if (mode == 1) {
        vec2 dir = dist > 0.0001 ? v_local / dist : vec2(1.0, 0.0);
        atten *= smoothstep(cos_outer, cos_inner, dir.x);
    }

    FragColor = vec4(color * atten, 1.0);
}
