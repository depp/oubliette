#version 120

attribute vec2 a_vert;
uniform vec4 u_vertxform;

void main() {
    vec2 vertscale = u_vertxform.xy;
    vec2 vertoff = u_vertxform.zw;
    gl_Position = vec4(a_vert * vertscale + vertoff, 0.0, 1.0);
}
