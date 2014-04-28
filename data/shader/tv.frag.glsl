#version 120

uniform sampler2D u_picture;
uniform sampler2D u_pattern;
uniform sampler1D u_banding;
uniform sampler2D u_noise;
uniform vec4 u_noiseoffset;
uniform vec2 u_texscale;
uniform vec4 u_color;
varying vec2 v_texcoord;

void main() {
    vec4 picture = texture2D(
        u_picture, v_texcoord * u_texscale);
    vec4 pattern = texture2D(
        u_pattern, v_texcoord * 0.25);
    vec4 banding = texture1D(
        u_banding, (v_texcoord.y + u_noiseoffset.z) * 0.0078125);
    vec4 noise = texture2D(
        u_noise, (v_texcoord + u_noiseoffset.xy) * 0.015625);
    noise *= 0.0625;
    banding = mix(banding, vec4(1.0, 1.0, 1.0, 1.0), 0.5);

    vec2 delta = v_texcoord - vec2(160.0, 90.0);
    float d = dot(delta, delta) * (2.967359050445104e-05);
    d = d * d;

    gl_FragColor =
        (picture * (1.0 - u_color.a * d) + u_color * d) *
        pattern * banding + noise;
}
