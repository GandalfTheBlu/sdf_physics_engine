#version 430

uniform vec3 u_color;

out vec4 o_color;

void main()
{
	o_color = vec4(u_color, 1.);
}