#include "cbuffers.hlsli"
#include "samplers.hlsli"
#include "blinn_phong.hlsli"

Texture2D diffuse_albedo_map : register(t0);

struct PSInput
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
    float3 normW : NORMAL;
};

float4 main(PSInput input) : SV_TARGET
{
    Material mat0 = mat;
    float2 uv = mul(float4(input.uv, 0, 1), mat.tex_tf).xy;
    float4 tex_diffuse_albedo = diffuse_albedo_map.Sample(samp_linear_wrap, uv);
    mat0.alpha *= tex_diffuse_albedo.w;
#ifdef ALPHA_CLIP
    clip(mat0.alpha - 0.1f);
#endif
    mat0.diffuse_albedo *= tex_diffuse_albedo.xyz;

    float3 normal = normalize(input.normW);
    float3 to_eye = eye_pos - input.posW;
    float dist_eye = length(to_eye);
    to_eye /= dist_eye;

    float3 color = 0;
    if (light_enable > 0.5f)
    {
        color += mat0.diffuse_albedo * light_ambient;
        for (int i = 0; i < 16; i++)
            color += ComputeLight(lights[i], mat0, normal, to_eye, input.posW);
    }
    color += mat0.emissive;
    float4 output = float4(saturate(color), mat0.alpha);

#ifdef FOG
    float s = saturate((dist_eye - fog_start) / (fog_end - fog_start));
    output = lerp(output, float4(fog_color, 1), s);
#endif

    return output;
}
