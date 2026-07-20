#version 410 core

layout (location = 0) out vec4 FragColor;

in vec2 v_uv;

uniform sampler2D source_texture;
uniform vec2 texel;

void main()
{
    vec3 sum = texture(source_texture, v_uv + texel * vec2(-1.0, -1.0)).rgb
             + texture(source_texture, v_uv + texel * vec2( 1.0, -1.0)).rgb
             + texture(source_texture, v_uv + texel * vec2(-1.0,  1.0)).rgb
             + texture(source_texture, v_uv + texel * vec2( 1.0,  1.0)).rgb;
    FragColor = vec4(sum * 0.25, 1.0);
}
