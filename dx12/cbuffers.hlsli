#ifndef _CBUFFERS_HLSLH_
#define _CBUFFERS_HLSLH_

#include "structs.hlsli"

cbuffer CBObj : register(b0)
{
    matrix world;
    matrix inv_world;
    matrix uv_tf;
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
cbuffer CBLight : register(b2)
{
    float light_enable; // 0: disable  1: enable
    float3 light_ambient;
    Light lights[16];
};

#endif
