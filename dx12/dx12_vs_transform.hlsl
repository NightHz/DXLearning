#include "cbuffers.hlsli"

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    float4 pos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(pos, view_proj);
    output.color = input.color;
    return output;
}
