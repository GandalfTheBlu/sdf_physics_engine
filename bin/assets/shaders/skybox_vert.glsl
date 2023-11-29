#version 430

layout(location=0) in vec3 a_position;

layout(location=0) out vec3 v_position;

uniform mat4 u_VP;

void main()
{
	v_position = a_position;
	gl_Position = u_VP * vec4(a_position, 1.);
}