#include "cbuffers.hlsli"
#include "blinn_phong.hlsli"

struct PSInput
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normW : NORMAL;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 normal = normalize(input.normW);
    float3 to_eye = normalize(eye_pos - input.posW);

    float3 color = 0;
    if (light_enable > 0.5f)
    {
        color += mat.diffuse_albedo * light_ambient;
        for (int i = 0; i < 16; i++)
            color += ComputeLight(lights[i], mat, normal, to_eye, input.posW);
    }
    color += mat.emissive;

    return float4(saturate(color), 1);
}
