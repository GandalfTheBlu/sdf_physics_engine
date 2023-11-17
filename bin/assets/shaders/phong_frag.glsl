#version 430

uniform vec3 u_color;
uniform vec3 u_camPos;
uniform vec3 u_lightDir;
uniform float u_roughness;

layout(location=0) in vec3 v_position;
layout(location=2) in vec3 v_normal;

out vec4 o_color;

void main()
{
	vec3 norm = normalize(v_normal);
	vec3 toPoint = v_position - u_camPos;
	vec3 ray = normalize(toPoint);
	float diff = max(0., dot(-u_lightDir, norm));
	float spec = pow(max(0., dot(reflect(ray, norm), -u_lightDir)), clamp(1.-u_roughness, 1./64., 1.) * 64.);
	vec3 amb = vec3(0.3, 0.3, 0.5);
	vec3 col = (vec3(diff + spec) + amb) * u_color;
	
	o_color = vec4(col, 1.);
}