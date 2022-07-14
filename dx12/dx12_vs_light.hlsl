#include "cbuffers.hlsli"
#include "blinn_phong.hlsli"

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

    float3x3 normal_world = { inv_world[0].xyz, inv_world[1].xyz, inv_world[2].xyz };
    float3 normal = normalize(mul(normal_world, input.norm));

    float4 pos = mul(float4(input.pos, 1), world);
    float3 to_eye = normalize(eye_pos - pos.xyz);

    float3 color = 0;
    if (light_enable > 0.5f)
    {
        color += mat.diffuse_albedo * light_ambient;
        for (int i = 0; i < 16; i++)
            color += ComputeLight(lights[i], mat, normal, to_eye, pos.xyz);
    }
    color += mat.emissive;

    output.pos = mul(pos, view_proj);
    output.color = saturate(float4(color, 1));

    return output;
}
