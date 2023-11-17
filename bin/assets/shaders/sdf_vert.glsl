#version 430

layout(location=0) in vec2 a_position;

layout(location=0) out vec2 v_position;

void main()
{
	v_position = a_position;
	gl_Position = vec4(a_position, 0., 1.);
}