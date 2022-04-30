cbuffer vscb_transform : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
    matrix world_view;
    matrix view_proj;
    matrix world_view_proj;
};

cbuffer vscb_material : register(b1)
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
    float power;
};

cbuffer vscb_light : register(b2)
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
    VSOutput output;
    output.pos = mul(input.pos, world_view_proj);
    output.color = diffuse;
    output.uv.x = 0;
    output.uv.y = 0.5f;

    float4 N = normalize(mul(input.norm, world_view)); // normal
    float4 cam = { 0,0,0,1 };
    float4 pos = mul(input.pos, world_view);
    float4 V = normalize(cam - pos); // to eye
    if (dl_enable)
    {
        float4 L = normalize(-mul(dl_dir, view)); // to light
        output.uv.x += max(0, dot(L, N));
    }
    if (pl_enable)
    {
        float4 L = mul(pl_pos, view) - pos;   // to light
        float I = pow(max(1, length(L) / pl_range), -2); // intensity
        L = normalize(L);
        output.uv.x += max(0, I * dot(L, N));
    }
    output.uv.x = clamp(output.uv.x, 0.01f, 0.99f);
    return output;
}
