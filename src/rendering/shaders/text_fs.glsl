#version 410 core

layout (location = 0) out vec4 FragColor;

in vec4 color;
in vec2 texture_coordinate;
flat in float texture_index;

uniform sampler2D texture_list[16];

void main()
{
    float distance;
    switch (int(texture_index)) {
        case 0: distance = texture(texture_list[0], texture_coordinate).r; break;
        case 1: distance = texture(texture_list[1], texture_coordinate).r; break;
        case 2: distance = texture(texture_list[2], texture_coordinate).r; break;
        case 3: distance = texture(texture_list[3], texture_coordinate).r; break;
        case 4: distance = texture(texture_list[4], texture_coordinate).r; break;
        case 5: distance = texture(texture_list[5], texture_coordinate).r; break;
        case 6: distance = texture(texture_list[6], texture_coordinate).r; break;
        case 7: distance = texture(texture_list[7], texture_coordinate).r; break;
        case 8: distance = texture(texture_list[8], texture_coordinate).r; break;
        case 9: distance = texture(texture_list[9], texture_coordinate).r; break;
        case 10: distance = texture(texture_list[10], texture_coordinate).r; break;
        case 11: distance = texture(texture_list[11], texture_coordinate).r; break;
        case 12: distance = texture(texture_list[12], texture_coordinate).r; break;
        case 13: distance = texture(texture_list[13], texture_coordinate).r; break;
        case 14: distance = texture(texture_list[14], texture_coordinate).r; break;
        case 15: distance = texture(texture_list[15], texture_coordinate).r; break;
        default: distance = 0.0;
    }

    float width = fwidth(distance);
    float alpha = smoothstep(0.5 - width, 0.5 + width, distance);
    FragColor = vec4(color.rgb, color.a * alpha);
}
