#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout(location = 2) in vec2 uv_in;
uniform mat4 vp;
uniform mat4 model;

out vec3 interpolated_normal;
out vec3 fragment_position;
out vec3 local_position;
out vec2 uv;

void main()
{
    interpolated_normal = mat3(model) * normal;
    vec4 world_pos = model * vec4(pos, 1.0);
    fragment_position = world_pos.xyz;
    local_position = pos;
    uv=uv_in;
    gl_Position = vp * world_pos;
}