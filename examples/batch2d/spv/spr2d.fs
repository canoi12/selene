#version 450
layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_texcoord;

layout(binding = 1) uniform sampler2D u_texture;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = v_color * texture(u_texture, v_texcoord);
}
