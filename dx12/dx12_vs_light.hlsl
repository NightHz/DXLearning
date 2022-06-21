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
    float3 norm : NORMAL;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    matrix normal_world = { inv_world[0],inv_world[1],inv_world[2],0,0,0,1 };
    float4 N = normalize(mul(normal_world, float4(input.norm, 0))); // normal

    float4 pos = mul(float4(input.pos, 1), world);
    float4 V = normalize(float4(eye_pos, 1) - pos); // to eye

    output.pos = mul(pos, view_proj);
    output.color = emissive;

    if (dl_enable)
    {
        float4 L = normalize(-dl_dir); // to light
        float4 R = normalize(reflect(-L, N)); // reflect

        output.color += ambient * dl_ambient;
        float s = max(0, dot(L, N));
        output.color += s * diffuse * dl_diffuse;
        if (dl_specular_enable)
            output.color += s * specular * dl_specular * (0.125f * power + 1) * pow(max(0, dot(R, V)), power);
    }
    if (pl_enable)
    {
        float4 L = pl_pos - pos; // to light
        float I = pow(max(1, length(L) / pl_range), -2); // intensity
        L = normalize(L);
        float4 R = normalize(reflect(-L, N)); // reflect

        output.color += I * ambient * pl_ambient;
        float s = max(0, dot(L, N));
        output.color += s * I * diffuse * dl_diffuse;
        if (pl_specular_enable)
            output.color += s * I * specular * dl_specular * pow(max(0, dot(R, V)), power);
    }
    output.color = saturate(output.color);

    return output;
}
