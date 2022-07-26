#include "cbuffers.hlsli"

struct VSInput
{
    float3 posL : POSITION;
    float2 uv : TEXCOORD;
    float3 normL : NORMAL;
};

struct VSOutput
{
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
    float3 normW : NORMAL;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    float4 pos = mul(float4(input.posL, 1.0f), world);
    output.posW = pos.xyz;
    float3x3 normal_world = { inv_world[0].xyz, inv_world[1].xyz, inv_world[2].xyz };
    output.normW = mul(normal_world, input.normL);
    output.uv = mul(float4(input.uv, 0, 1), uv_tf).xy;
    return output;
}
