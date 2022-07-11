#ifndef _BLINN_PHONG_HLSLH_
#define _BLINN_PHONG_HLSLH_

#include "registers.hlsli"

float CalcAttenuation(float x, float falloff_begin, float falloff_end)
{
	return saturate((falloff_end - x) / (falloff_end - falloff_begin));
}

float3 SchlickFresnel(float3 fresnel_r0, float dot_light_normal)
{
	float f0 = 1 - dot_light_normal;
	float3 reflect_percent = fresnel_r0 + (1 - fresnel_r0) * (f0 * f0 * f0 * f0 * f0);

	return reflect_percent;
}

float3 BlinnPhong(float3 light_intensity, float3 to_light, float3 normal, float3 to_eye, Material mat)
{
	float3 L = to_light;
	float3 N = normal;
	//float3 R = reflect(-L, N); // reflect direction
	float3 V = to_eye;
	float3 H = normalize(L + V); // half vector

	float si = saturate(dot(L, N)); // dot of to_light and normal
	float sh = saturate(dot(L, H));
	float sih = saturate(dot(H, N));
	float3 r = SchlickFresnel(mat.fresnel_r0, sh); // specular reflect percent
	float m = max((1 - saturate(mat.roughness)) * 256, 0.01f); // specular power
	float f = (0.125f * m + 1) * pow(sih, m); // specular factor

	float3 specular_albedo = r * f;
	//specular_albedo = specular_albedo / (specular_albedo + 1);
	float3 diffuse = si * light_intensity * mat.diffuse_albedo;
	float3 specular = si * light_intensity * specular_albedo;

	return diffuse + specular;
}

float3 ComputeDirectionalLight(Light light, Material mat, float3 normal, float3 to_eye)
{
	return BlinnPhong(light.intensity, normalize(-light.direction), normal, to_eye, mat);
}

float3 ComputePointLight(Light light, Material mat, float3 normal, float3 to_eye, float3 pos)
{
	float3 to_light = light.position - pos;
	float d = length(to_light); // distance
	if (d > light.falloff_end)
		return 0;
	else
	{
		to_light /= d;

		float3 light_intensity = CalcAttenuation(d, light.falloff_begin, light.falloff_end) * light.intensity;

		return BlinnPhong(light_intensity, to_light, normal, to_eye, mat);
	}
}

float3 ComputeSpotLight(Light light, Material mat, float3 normal, float3 to_eye, float3 pos)
{
	float3 to_light = light.position - pos;
	float d = length(to_light); // distance
	if (d > light.falloff_end)
		return 0;
	else
	{
		to_light /= d;
		float cosphi = saturate(dot(normalize(light.direction), -to_light));
		if (cosphi == 0)
			return 0;
		else
		{
			float f1 = CalcAttenuation(d, light.falloff_begin, light.falloff_end); // attenuation
			float m = (1 - saturate(light.spot_divergence)) * 32 + 1; // spot power
			float f2 = (0.125f * m + 1) * pow(cosphi, m); // spot factor

			float3 light_intensity = (f1 * f2) * light.intensity;

			return BlinnPhong(light_intensity, to_light, normal, to_eye, mat);
		}
	}
}

float3 ComputeLight(Light light, Material mat, float3 normal, float3 to_eye, float3 pos)
{
	if (light.type < 0.5f)
		return 0;
	else if (light.type < 1.5f)
		return ComputeDirectionalLight(light, mat, normal, to_eye);
	else if (light.type < 2.5f)
		return ComputePointLight(light, mat, normal, to_eye, pos);
	else if (light.type < 3.5f)
		return ComputeSpotLight(light, mat, normal, to_eye, pos);
	else
		return 0;
}

#endif
