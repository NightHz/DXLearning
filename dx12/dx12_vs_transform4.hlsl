#include "cbuffers.hlsli"

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
    VSOutput output;
    float4 pos = mul(float4(input.posL, 1.0f), world);
    output.posW = pos.xyz;
    output.posH = mul(pos, view_proj);
    float3x3 normal_world = { inv_world[0].xyz, inv_world[1].xyz, inv_world[2].xyz };
    output.normW = normalize(mul(normal_world, input.normL));
    output.color = input.color;
    output.uv = input.uv;
    return output;
}
