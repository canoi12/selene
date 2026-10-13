#version 450

// Selene canonical source — sprite2d fragment stage.

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_texcoord;

// Explicit binding = 0: Vulkan/SPIR-V requires a binding on samplers, and on
// OpenGL this maps the sampler to texture unit 0 -- which is where the engine
// binds textures (set_texture slot defaults to 0). Do not use binding = 1 here.
layout(binding = 0) uniform sampler2D u_texture;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = v_color * texture(u_texture, v_texcoord);
}
