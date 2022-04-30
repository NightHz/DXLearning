cbuffer vscb_transform : register(b0)
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
    float4 pos : POSITION;
    float4 norm : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float2 uv2 : TEXCOORD1;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    float4 black = { 0,0,0,1 };
    float expand = 0.06f;
    VSOutput output;
    output.pos = mul(input.pos + expand * input.norm, world_view_proj);
    output.color = black;
    output.uv = input.uv;
    return output;
}
