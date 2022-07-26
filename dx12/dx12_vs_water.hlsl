#include "cbuffers.hlsli"
#include "water_wave.hlsli"

struct VSInput
{
    float3 posL : POSITION;
    float3 normL : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normW : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    float4 pos = mul(float4(input.posL, 1.0f), world);
    float2 partial_xz;
    float offset = get_water_offset(pos.xz, time, partial_xz);
    float4 up = mul(float4(0, 1, 0, 0), world);
    pos += up * offset;
    float3x3 normal_world = { inv_world[0].xyz, inv_world[1].xyz, inv_world[2].xyz };
    float3 norm = mul(normal_world, float3(0, 1, 0));
    norm += float3(-partial_xz.x, 0, -partial_xz.y);

    VSOutput output;
    output.posW = pos.xyz;
    output.posH = mul(pos, view_proj);
    output.normW = normalize(norm);
    output.color = input.color;
    float2 uv = input.uv + float2(1, 1) * 0.03f * time;
    output.uv = mul(float4(uv, 0, 1), uv_tf).xy;
    return output;
}
