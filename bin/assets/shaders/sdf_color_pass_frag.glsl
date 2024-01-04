#version 430

#define MAX_DISTANCE 500.

uniform mat4 u_VP;
uniform mat4 u_invVP;
layout(binding=0) uniform sampler2D u_distanceTexture;

layout(location=0) in vec2 v_position;

out vec4 o_color;
out float gl_FragDepth;

void ScreenRay(inout vec3 origin, inout vec3 ray)
{
	vec4 nearPos = u_invVP * vec4(v_position.x, v_position.y, -1., 1.);
	vec4 farPos = u_invVP * vec4(v_position.x, v_position.y, 1., 1.);
	nearPos /= nearPos.w;
	farPos /= farPos.w;
	origin = nearPos.xyz;
	ray = normalize(vec3(farPos - nearPos));
}


float SmoothUnion(float d1, float d2, float k) 
{
    float h = clamp(0.5 + 0.5*(d2-d1)/k, 0., 1.);
    return mix(d2, d1, h) - k*h*(1.-h); 
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

float Tree(vec3 p)
{	
	vec3 q = p;

	vec2 dim = vec2(1., 8.);
	float d = Capsule(p, vec3(0., -1., 0.), vec3(0., 1. + dim.y, 0.), dim.x);
	vec3 scale = vec3(1.);
	vec3 change = vec3(0.7,0.68,0.7);
	
	vec3 n1 = normalize(vec3(1., 0., 1.)); 
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
		
		d = SmoothUnion(d, Capsule(p, vec3(0.), vec3(0., dim.y * scale.y, 0.), scale.x*dim.x), 0.4);
	}
	
	return d;
}

//float Sdf(vec3 p)
//{
//	float plane = p.y - 0.3 * sin(p.x * 0.5) * sin(p.z * 0.5);
//	float trees = Tree(RepXZ(p, 30., 30.));
//	return min(plane, trees);
//}

__SDF__


float OffsetError(float t)
{
	return 0.001 * t;
}

vec3 AdjustPoint(vec3 origin, vec3 ray, float t)
{
	for(int i=0; i<3; i++)
	{
		t += Sdf(origin + ray * t) - OffsetError(t);
	}
	
	return origin + ray * t;
}

vec3 CalcNormal(vec3 p)
{
    const float h = 0.0001;
    const vec2 k = vec2(1.,-1.);
    return normalize(k.xyy*Sdf(p + k.xyy*h) + 
                     k.yyx*Sdf(p + k.yyx*h) + 
                     k.yxy*Sdf(p + k.yxy*h) + 
                     k.xxx*Sdf(p + k.xxx*h));
}

float Shadow(vec3 origin, vec3 ray, float minT, float maxT)
{
	float t = minT;
	
    for(int i=0; i<100 && t<maxT; i++)
    {
        float r = Sdf(origin + ray * t);
		
        if(r < 0.01)
		{
            return 0.;
		}
		
        t += r;
    }
	
    return 1.;
}


void main()
{
	vec2 uv = vec2(0.5) + v_position * 0.5;
	float t = texture(u_distanceTexture, uv).r;
	
	if(t > MAX_DISTANCE)
		discard;
	
	vec3 origin = vec3(0.);
	vec3 ray = vec3(0.);
	ScreenRay(origin, ray);
	
	vec3 point = AdjustPoint(origin, ray, t);//origin + ray * t;
	vec4 clipPoint = u_VP * vec4(point, 1.);
	gl_FragDepth = (clipPoint.z / clipPoint.w + 1.) * 0.5;
	
	vec3 normal = CalcNormal(point);
	vec3 reflected = reflect(ray, normal);
	vec3 lightDir = normalize(vec3(0.5, -1., 0.8));
	vec3 amb = vec3(0.4, 0.4, 0.5);
	float shadow = Shadow(point + normal * 0.01, -lightDir, 0.3, 100.);
	float diff = max(0., dot(normal, -lightDir));
	float spec = pow(max(0., dot(reflected, -lightDir)), 16.);
	vec3 tint = mix(vec3(0.5, 0.4, 0.3), vec3(0.8, 0.8, 1.), min(normal.y, 1.));
	
	vec3 col = tint * (amb + vec3(shadow * (diff + spec)));
	
	o_color = vec4(col, 1.);
}