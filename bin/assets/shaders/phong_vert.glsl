#version 430

layout(location=0) in vec3 a_position;
layout(location=2) in vec3 a_normal;

layout(location=0) out vec3 v_position;
layout(location=2) out vec3 v_normal;

uniform mat4 u_MVP;
uniform mat4 u_M;
uniform mat3 u_N;

void main()
{
	v_normal = u_N * a_normal;
	v_position = (u_M * vec4(a_position, 1.)).xyz;
	gl_Position = u_MVP * vec4(a_position, 1.);
}