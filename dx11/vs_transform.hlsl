cbuffer vscb_transform : register(b0)
{
    matrix transform_world;
    matrix transform_view;
    matrix transform_proj;
    matrix transform_world_view;
    matrix transform_view_proj;
    matrix transform_world_view_proj;
};

struct VSInput
{
    float4 pos : POSITION;
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
    output.pos = mul(input.pos, transform_world_view_proj);
    output.color = input.color;
    return output;
}