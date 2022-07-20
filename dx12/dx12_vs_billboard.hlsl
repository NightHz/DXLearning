#include "cbuffers.hlsli"

struct VSInput
{
    float3 posL : POSITION;
    float2 sizeL : SIZE;
};

struct VSOutput
{
    float3 posW : POSITION;
    float2 sizeW : SIZE;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.posW = mul(float4(input.posL, 1), world).xyz;
    output.sizeW = mul(float4(input.sizeL, 0, 0), world).xy;
    return output;
}
