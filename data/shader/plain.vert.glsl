#version 120

attribute vec2 a_vert;
uniform mat2 u_vertmat;
uniform vec2 u_vertoff;

void main() {
    gl_Position = vec4(u_vertmat * a_vert + u_vertoff, 0.0, 1.0);
}
