#version 150

varying vec4 v_color;
varying vec2 v_texcoord;

uniform sampler2D u_texture;

void main() {
    gl_FragColor = v_color * texture(u_texture, v_texcoord);
}