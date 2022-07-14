#include "registers.hlsli"
#include "blinn_phong.hlsli"

Texture2D diffuse_albedo_map : register(t0);
SamplerState samp : register(s0);

struct PSInput
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normW : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 normal = normalize(input.normW);
    float3 to_eye = normalize(eye_pos - input.posW);

    Material mat0 = mat;
    float4 tex_diffuse_albedo = diffuse_albedo_map.Sample(samp, input.uv);
    mat0.diffuse_albedo *= tex_diffuse_albedo.xyz;
    mat0.alpha *= tex_diffuse_albedo.w;

    float3 color = 0;
    if (light_enable > 0.5f)
    {
        color += mat0.diffuse_albedo * light_ambient;
        for (int i = 0; i < 16; i++)
            color += ComputeLight(lights[i], mat0, normal, to_eye, input.posW);
    }
    color += mat0.emissive;

    return float4(saturate(color), 1);
}
