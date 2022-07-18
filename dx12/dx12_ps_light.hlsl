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
    float3 to_eye = eye_pos - input.posW;
    float dist_eye = length(to_eye);
    to_eye /= dist_eye;

    float3 color = 0;
    if (light_enable > 0.5f)
    {
        color += mat.diffuse_albedo * light_ambient;
        for (int i = 0; i < 16; i++)
            color += ComputeLight(lights[i], mat, normal, to_eye, input.posW);
    }
    color += mat.emissive;
    float4 output = float4(saturate(color), mat.alpha);

#ifndef DISABLE_FOG
    float s = saturate((dist_eye - fog_start) / (fog_end - fog_start));
    output = lerp(output, float4(fog_color, 1), s);
#endif

    return output;
}
