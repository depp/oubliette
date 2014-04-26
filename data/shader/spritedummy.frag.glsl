#version 120

varying vec2 v_texcoord;

// This models a texture which is:
// red at the lower left,
// yellow at the lower right,
// green at the upper right,
// blue at the upper left.
void main() {
    gl_FragColor = mix(
        mix(vec4(0, 0, 1, 0), vec4(0, 1, 0, 1), v_texcoord.x),
        mix(vec4(1, 0, 0, 1), vec4(1, 1, 0, 1), v_texcoord.x),
        v_texcoord.y);
}
