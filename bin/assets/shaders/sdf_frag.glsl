#version 430

uniform mat4 u_invVP;
uniform vec3 u_camPos;
uniform float u_pixelRadius;
uniform vec4 u_spherePosRadius;

layout(location=0) in vec2 v_position;

out vec4 o_color;

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
    float h = clamp(0.5 + 0.5*(d2-d1)/k, 0.0, 1.0);
    return mix(d2, d1, h) - k*h*(1.0-h); 
}

float Sdf(vec3 p)
{
	vec3 q = p - u_spherePosRadius.xyz;
	float sphere = length(q) - u_spherePosRadius.w;
	float plane = p.y + 0.3 * sin(p.x) * sin(p.z);
	return SmoothUnion(sphere, plane, 2.);
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
    for(float i=0.; i<100. && t<maxT; i++)
    {
        float h = Sdf(origin + ray * t);
        if(h<0.001)
            return 0.0;
		
        res = min(res, k*h/t);
        t += h;
    }
	
    return res;
}

float RayMarch(vec3 origin, vec3 ray)
{
	const float infinity = 100000.;
	const float minT = 0.3;
	const float maxT = 100.;
	
	float stepScale = 1.2;
	float prevRadius = 0.;
	float stepLength = 0.;
	float t = minT;
	float candidateT = minT;
	float candidateError = infinity;
	
	for(float i=0.; i<100.; i++)
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
	
	vec3 lightDir = normalize(vec3(-0.5, -1., 0.8));
	vec3 col = vec3(0.);
	
	if(t < 100.)
	{
		vec3 point = origin + ray * t;
		vec3 normal = CalcNormal(point);
		float diff = max(0., dot(normal, -lightDir));
		float spec = pow(max(0., dot(reflect(ray, normal), -lightDir)), 32.);
		float shadow = SoftShadow(point + normal * 0.01, -lightDir, 0.3, 100., 8.);
		vec3 f = fract(point);
		float pattern = exp(-0.2*length(min(f, 1.-f)));
		
		col = vec3((diff + spec) * pattern * shadow);
	}
	
	o_color = vec4(col, 1.);
}