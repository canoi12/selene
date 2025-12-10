#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texcoord;

layout (binding = 0) uniform Matrices {
    mat4 world;
};


layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_texcoord;

void main() {
    gl_Position = world * vec4(a_position, 1.0);
    v_color = a_color;
    v_texcoord = a_texcoord;
}
