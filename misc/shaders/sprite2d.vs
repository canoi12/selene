#version 450

// Selene canonical source — sprite2d vertex stage.
// Compiled to SPIR-V (Vulkan / D3D12) and transpiled to HLSL (D3D11) or GLSL ES
// by misc/shaders/build_shaders.py. Keep attribute locations and bindings stable:
// the engine derives the vertex layout and resource bindings from SPIR-V reflection.

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texcoord;

layout(binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
};

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_texcoord;

void main() {
    gl_Position = projection * view * vec4(a_position, 1.0);
    v_color = a_color;
    v_texcoord = a_texcoord;
}
