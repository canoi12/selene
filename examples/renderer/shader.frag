#version 450

layout (location = 0) in vec4 v_color;
layout (location = 1) in vec2 v_texcoord;

layout (location = 0) out vec4 o_color;

layout (binding = 0) uniform sampler2D u_texture;

void main() {
    o_color = v_color * texture(u_texture, v_texcoord);
}
