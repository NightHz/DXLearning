cbuffer CBTransform : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
    matrix world_view;
    matrix view_proj;
    matrix world_view_proj;
};

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
    output.pos = mul(float4(input.pos, 1.0f), world_view_proj);
    output.color = input.color;
    return output;
}
