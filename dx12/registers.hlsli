#ifndef _REGISTERS_HLSLH_
#define _REGISTERS_HLSLH_

struct Material
{
    float3 diffuse_albedo;
    float alpha;
    float3 fresnel_r0; // reflect percent when theta = 0
    float roughness;
    float3 emissive;
    float _pad1;
};
cbuffer CBObj : register(b0)
{
    matrix world;
    matrix inv_world;
    Material mat;
};
cbuffer CBFrame : register(b1)
{
    matrix view;
    matrix inv_view;
    matrix proj;
    matrix view_proj;
    float3 eye_pos;
    float3 eye_at;
    float2 screen_size;
    float time;
    float deltatime;
};
struct Light
{
    float type; // 0: disable  1: directional light  2: point light  3: spot light
    float3 intensity;
    float3 direction; // for directional light and spot light
    float falloff_begin; // for point light and spot light
    float falloff_end; // for point light and spot light
    float3 position; // for point light and spot light
    float spot_divergence; // for spot light
    float3 _pad1;
};
cbuffer CBLight : register(b2)
{
    float light_enable; // 0: disable  1: enable
    float3 light_ambient;
    Light lights[16];
};

#endif
