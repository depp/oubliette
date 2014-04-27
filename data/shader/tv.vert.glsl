#version 120

attribute vec4 a_vert;
varying vec2 v_texcoord;

void main() {
    v_texcoord = a_vert.zw;
    gl_Position = vec4(a_vert.xy, 0.0, 1.0);
}
