#version 450 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex_coord;
layout (location = 3) in vec3 a_tangent;

out vec2 texture_coordinate;
out mat3 TBN;
out vec3 light_position_tangent;
out vec3 viewer_position_tangent;
out vec3 fragment_position_tangent;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewer_position;
uniform Light light;

void main()
{
    gl_Position = projection * view * model * vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);

    texture_coordinate = a_tex_coord;

    // re-orthogonalize T with respect to N
    vec3 T = normalize(vec3(model * vec4(a_tangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(a_norm, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));

    //Send in tangent space vectors for light calculation
    light_position_tangent = TBN * light.position;
    viewer_position_tangent = TBN * viewer_position;
    fragment_position_tangent = TBN * vec3(model * vec4(a_pos, 1.0));
}