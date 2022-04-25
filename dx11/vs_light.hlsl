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
    float dl_enable;
    float dl_specular_enable;
    float4 dl_dir;
    float4 dl_ambient;
    float4 dl_diffuse;
    float4 dl_specular;
    float pl_enable;
    float pl_range;
    float pl_specular_enable;
    float4 pl_pos;
    float4 pl_ambient;
    float4 pl_diffuse;
    float4 pl_specular;
};

struct VSInput
{
    float4 pos : POSITION;
    //float4 norm : NORMAL;
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
    output.pos = mul(input.pos, world_view_proj);
    output.color = 0;
    if (dl_enable == 1)
    {
        output.color += ambient * dl_ambient;
        output.color += diffuse * dl_diffuse * input.color * 0.3f;
    }
    if (pl_enable == 1)
    {

    }
    output.color = clamp(output.color, 0, 1);
    return output;
}
