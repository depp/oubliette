#version 120

attribute vec4 a_vert;
uniform vec4 u_vertxform;
uniform vec2 u_texscale;
varying vec2 v_texcoord;

void main() {
    vec2 vertscale = u_vertxform.xy;
    vec2 vertoff = u_vertxform.zw;
    v_texcoord = a_vert.zw * u_texscale;
    gl_Position = vec4(a_vert.xy * vertscale + vertoff, 0.0, 1.0);
}
