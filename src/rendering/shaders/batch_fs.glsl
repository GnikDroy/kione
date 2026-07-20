#version 410 core

layout (location = 0) out vec4 FragColor;

in vec4 color;
in vec2 texture_coordinate;
flat in float texture_index;

uniform sampler2D texture_list[16];

void main()
{
    vec4 out_color;
    switch (int(texture_index)) {
        case 0: out_color = color * texture(texture_list[0], texture_coordinate); break;
        case 1: out_color = color * texture(texture_list[1], texture_coordinate); break;
        case 2: out_color = color * texture(texture_list[2], texture_coordinate); break;
        case 3: out_color = color * texture(texture_list[3], texture_coordinate); break;
        case 4: out_color = color * texture(texture_list[4], texture_coordinate); break;
        case 5: out_color = color * texture(texture_list[5], texture_coordinate); break;
        case 6: out_color = color * texture(texture_list[6], texture_coordinate); break;
        case 7: out_color = color * texture(texture_list[7], texture_coordinate); break;
        case 8: out_color = color * texture(texture_list[8], texture_coordinate); break;
        case 9: out_color = color * texture(texture_list[9], texture_coordinate); break;
        case 10: out_color = color * texture(texture_list[10], texture_coordinate); break;
        case 11: out_color = color * texture(texture_list[11], texture_coordinate); break;
        case 12: out_color = color * texture(texture_list[12], texture_coordinate); break;
        case 13: out_color = color * texture(texture_list[13], texture_coordinate); break;
        case 14: out_color = color * texture(texture_list[14], texture_coordinate); break;
        case 15: out_color = color * texture(texture_list[15], texture_coordinate); break;
        default :
        out_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    FragColor = out_color;
}
