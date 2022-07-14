#ifndef _STRUCTS_HLSLH_
#define _STRUCTS_HLSLH_

struct Material
{
    float3 diffuse_albedo;
    float alpha;
    float3 fresnel_r0; // reflect percent when theta = 0
    float roughness;
    float3 emissive;
    float _pad1;
    matrix tex_tf;
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

#endif
