#version 430

#define MAX_DISTANCE 500.
#define INFINITY 100000.

uniform mat4 u_invVP;
uniform float u_textureDiagonal;
uniform int u_isFirstPass;
uniform int u_iterationCount; 
uniform float u_time;
layout(binding=0) uniform sampler2D u_distanceTexture;

layout(location=0) in vec2 v_position;

out float o_distance;

void ScreenRay(inout vec3 origin, inout vec3 ray)
{
	vec4 nearPos = u_invVP * vec4(v_position.x, v_position.y, -1., 1.);
	vec4 farPos = u_invVP * vec4(v_position.x, v_position.y, 1., 1.);
	nearPos /= nearPos.w;
	farPos /= farPos.w;
	origin = nearPos.xyz;
	ray = normalize(vec3(farPos - nearPos));
}

vec4 Union(vec4 a, vec4 b)
{
	if(a.w < b.w)
		return a;
	
	return b;
}

vec4 Cut(vec4 a, vec4 b)
{
	b.w = -b.w;
	if(a.w > b.w)
		return a;
	
	return b;
}

vec4 Intersect(vec4 a, vec4 b)
{
	if(a.w > b.w)
		return a;
	
	return b;
}

vec4 SmoothUnion(vec4 d1, vec4 d2, float k)
{
    float h = clamp(0.5 + 0.5 * (d2.w - d1.w) / k, 0., 1.);
	vec4 d = mix(d2, d1, h);
    return vec4(d.xyz, d.w - (k * h * (1. - h)));
}

vec3 RepXZ(vec3 p, float x, float y)
{
	vec2 q = vec2(x, y) * round(p.xz / vec2(x, y));
	return p - vec3(q.x, 0., q.y);
}

//float Box(vec3 p, vec3 b)
//{
//	vec3 q = abs(p) - b;
//	return length(max(q, 0.)) + min(max(q.x, max(q.y, q.z)), 0.);
//}

float Capsule(vec3 p, vec3 a, vec3 b, float r)
{
	vec3 pa = p - a;
	vec3 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
	return length(pa - ba * h) - r;
}

vec3 Fold(vec3 p, vec3 n)
{
	return p - 2.*min(0., dot(p, n)) * n;
}

vec3 RotX(vec3 p, float k)
{
	float k0 = atan(p.y, p.z);
	float r = length(p.yz);
	return vec3(p.x, r*sin(k0+k), r*cos(k0+k));
}

float Time()
{
	return u_time;
}

vec2 Tree(vec3 p)
{	
	vec3 q = p;

	vec2 dim = vec2(1., 8.);
	float d = Capsule(p, vec3(0., -1., 0.), vec3(0., 1. + dim.y, 0.), dim.x);
	vec3 scale = vec3(1.);
	vec3 change = vec3(0.7,0.68,0.7);
	float itr = 0.;
	
	vec3 n1 = normalize(vec3(1., 0., 1. + 0.1 * cos(Time()))); 
	vec3 n2 = vec3(n1.x, 0., -n1.z);
	vec3 n3 = vec3(-n1.x, 0., n1.z);
	
	for(int i=0; i<7; i++)
	{
		p = Fold(p, n1);	
		p = Fold(p, n2);	
		p = Fold(p, n3);	
		
		p.y -= scale.y*dim.y;
		p.z = abs(p.z);
		p = RotX(p, 3.1415*0.25);
		
		scale *= change;
		
		float d2 = Capsule(p, vec3(0.), vec3(0., dim.y * scale.y, 0.), scale.x*dim.x);
		if(d2 < d)
		{
			d = d2;
			itr = float(i);
		}
	}
	
	return vec2(itr / 7., d);
}

__SDF__

float RayMarch(vec3 origin, vec3 ray)
{
	const float minRScale = sqrt(2.) / u_textureDiagonal;
	
	float t = u_isFirstPass == 1 ? 0. : 
		texture(u_distanceTexture, vec2(0.5) + v_position * 0.5).r;
	
	if(t > MAX_DISTANCE)
		return t;
	
	float rPrev = 0.;
	float rCurr = 0.;
	float rNext = INFINITY;
	float stepScale = 0.9;
	float step = 0.;
	
	for(int i=0; i<u_iterationCount; i++)
	{
		float denom = step + rPrev - rCurr;
		if(denom != 0.)
		{
			step = abs(rCurr + stepScale * rCurr * 
				(step - rPrev + rCurr) / denom);
		}
		
		rNext = Sdf(origin + ray * (t + step)).w;
		
		if(step > rCurr + rNext)
		{
			step = rCurr;
			rNext = Sdf(origin + ray * (t + step)).w;
		}
		
		t += step;
		rPrev = rCurr;
		rCurr = rNext;
		
		if(rNext < minRScale * t || t > MAX_DISTANCE)
			break;
	}
	
	return t;
}

void main()
{
	vec3 origin = vec3(0.);
	vec3 ray = vec3(0.);
	ScreenRay(origin, ray);
	o_distance = RayMarch(origin, ray);
}