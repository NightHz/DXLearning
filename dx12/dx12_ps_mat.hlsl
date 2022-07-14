#include "cbuffers.hlsli"

float4 main() : SV_TARGET
{
    return float4(saturate(mat.diffuse_albedo + mat.emissive), 1);
}
