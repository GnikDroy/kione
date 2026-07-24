#version 410 core

layout (location = 0) out vec4 FragColor;

in vec4 color;
in vec2 texture_coordinate;
flat in float texture_index;

uniform sampler2D texture_list[16];

void main()
{
    vec4 tex;
    switch (int(texture_index)) {
        case 0: tex = texture(texture_list[0], texture_coordinate); break;
        case 1: tex = texture(texture_list[1], texture_coordinate); break;
        case 2: tex = texture(texture_list[2], texture_coordinate); break;
        case 3: tex = texture(texture_list[3], texture_coordinate); break;
        case 4: tex = texture(texture_list[4], texture_coordinate); break;
        case 5: tex = texture(texture_list[5], texture_coordinate); break;
        case 6: tex = texture(texture_list[6], texture_coordinate); break;
        case 7: tex = texture(texture_list[7], texture_coordinate); break;
        case 8: tex = texture(texture_list[8], texture_coordinate); break;
        case 9: tex = texture(texture_list[9], texture_coordinate); break;
        case 10: tex = texture(texture_list[10], texture_coordinate); break;
        case 11: tex = texture(texture_list[11], texture_coordinate); break;
        case 12: tex = texture(texture_list[12], texture_coordinate); break;
        case 13: tex = texture(texture_list[13], texture_coordinate); break;
        case 14: tex = texture(texture_list[14], texture_coordinate); break;
        case 15: tex = texture(texture_list[15], texture_coordinate); break;
        default :
        tex = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    FragColor = vec4(color.rgb * color.a, color.a) * tex;
}
