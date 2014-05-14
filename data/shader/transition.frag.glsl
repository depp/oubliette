#version 120

uniform sampler2D u_picture1;
uniform sampler2D u_picture2;
uniform vec2 u_center;
uniform float u_time;
uniform float u_maxradius;
varying vec2 v_texcoord;

void main()
{
	float dist = dot(abs(u_texcoord - u_center), vec2(1.0, 1.0));
}
