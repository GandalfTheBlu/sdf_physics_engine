#version 430

#define MAX_SPHERES 20
#define MAX_CAPSULES 10
#define MAX_DISTANCE 100.

uniform vec2 u_nearFar;
uniform mat4 u_invVP;
uniform vec3 u_camPos;
uniform float u_pixelRadius;
uniform vec4 u_spheres[MAX_SPHERES];
uniform int u_sphereCount;
uniform vec4 u_capsules[MAX_CAPSULES * 2];
uniform int u_capsuleCount;

layout(location=0) in vec2 v_position;

out vec4 o_color;
out float gl_FragDepth;

vec3 ScreenRay()
{
	vec4 nearPos = u_invVP * vec4(v_position.x, v_position.y, -1.f, 1.f);
	vec4 farPos = u_invVP * vec4(v_position.x, v_position.y, 1.f, 1.f);
	nearPos /= nearPos.w;
	farPos /= farPos.w;
	return normalize(vec3(farPos - nearPos));
}

float SmoothUnion(float d1, float d2, float k) 
{
    float h = clamp(0.5 + 0.5*(d2-d1)/k, 0., 1.);
    return mix(d2, d1, h) - k*h*(1.-h); 
}

float Capsule(vec3 p, vec3 a, vec3 b, float r)
{
	vec3 pa = p - a;
	vec3 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
	return length(pa - ba * h) - r;
}

float Box(vec3 p, vec3 b)
{
	vec3 q = abs(p) - b;
	return length(max(q, 0.)) + min(max(q.x, max(q.y, q.z)), 0.);
}

vec3 RepXZ(vec3 p, vec2 s)
{
	vec2 q = s * round(p.xz / s);
	return p - vec3(q.x, 0., q.y);
}

vec3 RotX(vec3 p, float k)
{
	float k0 = atan(p.y, p.z);
	float r = length(p.yz);
	return vec3(p.x, r*sin(k0+k), r*cos(k0+k));
}

vec3 Fold(vec3 p, vec3 n)
{
	return p - 2.*min(0., dot(p, n)) * n;
}

float Tree(vec3 p)
{	
	vec2 dim = vec2(1., 8.);
	float d = Capsule(p, vec3(0., -1., 0.), vec3(0., 1. + dim.y, 0.), dim.x);
	vec3 scale = vec3(1.);
	vec3 change = vec3(0.6,0.75,0.6);
	
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
		
		d = min(d, Capsule(p, vec3(0.), vec3(0., dim.y * scale.y, 0.), scale.x*dim.x));
	}
	
	return d;
}

float Sdf(vec3 p)
{
	float d = 100000.;
	
	for(int i=0; i<u_sphereCount; i++)
		d = min(d, distance(u_spheres[i].xyz, p) - u_spheres[i].w);
	
	for(int i=0; i+1<u_capsuleCount * 2; i+=2)
	{
		vec4 aAndR = u_capsules[i];
		vec3 b = u_capsules[i+1].xyz;
		d = min(d, Capsule(p, aAndR.xyz, b, aAndR.w));
	}
	
	float plane = p.y;
	float trees = Tree(RepXZ(p-vec3(0.,1.,0.), vec2(40.)));
	plane = min(plane, trees);
	
	return SmoothUnion(d, plane, 0.6);
}

vec3 CalcNormal(vec3 p)
{
    const float h = 0.0001;
    const vec2 k = vec2(1,-1);
    return normalize(k.xyy*Sdf(p + k.xyy*h) + 
                     k.yyx*Sdf(p + k.yyx*h) + 
                     k.yxy*Sdf(p + k.yxy*h) + 
                     k.xxx*Sdf(p + k.xxx*h));
}

float SoftShadow(vec3 origin, vec3 ray, float minT, float maxT, float k)
{
    float res = 1.0;
    float t = minT;
    for(int i=0; i<100 && t<maxT; i++)
    {
        float h = Sdf(origin + ray * t);
        if(h<0.01)
            return 0.0;
		
		//res = min(res, k*h/t);
        t += h;
    }
	
    return res;
}

float RayMarch(vec3 origin, vec3 ray)
{
	const float infinity = 100000.;
	const float minT = 0.3;
	const float maxT = MAX_DISTANCE;
	
	float stepScale = 1.2;
	float prevRadius = 0.;
	float stepLength = 0.;
	float t = minT;
	float candidateT = minT;
	float candidateError = infinity;
	
	for(int i=0; i<100; i++)
	{
		float signedRadius = Sdf(origin + ray * t);
		float radius = abs(signedRadius);
		
		bool longStepFail = stepScale > 1. && (radius + prevRadius) < stepLength;
		
		if(longStepFail)
		{
			stepLength -= stepScale * stepLength;
			stepScale = 1.;
		}
		else
		{
			stepLength = signedRadius * stepScale;
		}
		
		prevRadius = radius;
		float error = radius / t;
		
		if(!longStepFail && error < candidateError)
		{
			candidateT = t;
			candidateError = error;
		}
		
		if((!longStepFail && error < u_pixelRadius) || t > maxT)
			break;
		
		t += stepLength;
	}
	
	if(t > maxT || candidateError > u_pixelRadius)
		return infinity;
	
	return candidateT;
}

void main()
{
	vec3 origin = u_camPos;
	vec3 ray = ScreenRay();
	float t = RayMarch(origin, ray);
	
	if(t >= MAX_DISTANCE)
		discard;
	
	float zNear = u_nearFar.x;
	float zFar = u_nearFar.y;
	float lin = t;
	gl_FragDepth = (1. + (2. * zNear * zFar / lin - zFar - zNear) / (zNear - zFar)) / 2.;
	
	vec3 point = origin + ray * t;
	vec3 normal = CalcNormal(point);
	vec3 reflected = reflect(ray, normal);
	vec3 lightDir = normalize(vec3(0.5, -1., 0.8));
	vec3 amb = vec3(0.4, 0.4, 0.5);
	float shadow = SoftShadow(point + normal * 0.01, -lightDir, 0.3, 100., 8.);
	float diff = max(0., dot(normal, -lightDir));
	float spec = pow(max(0., dot(reflected, -lightDir)), 16.);
	vec3 tint = mix(vec3(0.7, 0.6, 0.5), vec3(0.5, 0.6, 0.7), min(point.y, 1.));
	vec3 col = tint * (amb + vec3(shadow * (diff + spec)));
	
	o_color = vec4(col, 1.);
}