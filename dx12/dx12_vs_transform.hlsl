cbuffer CBObj : register(b0)
{
    matrix world;
    matrix inv_world;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
    float power;
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
    bool dl_enable;
    bool dl_specular_enable;
    float4 dl_dir;
    float4 dl_ambient;
    float4 dl_diffuse;
    float4 dl_specular;
    bool pl_enable;
    bool pl_specular_enable;
    float pl_range;
    float4 pl_pos;
    float4 pl_ambient;
    float4 pl_diffuse;
    float4 pl_specular;
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
    output.pos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(output.pos, view_proj);
    output.color = input.color;
    return output;
}
