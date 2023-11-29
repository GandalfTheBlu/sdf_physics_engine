#version 430

layout(binding=0) uniform samplerCube u_skybox;

layout(location=0) in vec3 v_position;

out vec4 o_color;

void main()
{
	o_color = vec4(texture(u_skybox, v_position).rgb, 1.);
}