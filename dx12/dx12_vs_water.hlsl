#include "cbuffers.hlsli"

struct VSInput
{
    float3 posL : POSITION;
    float3 normL : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normW : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PlaneWave
{
    float2 dir;
    float wavelength;
    float speed;
    float amplitude;
    float phase0;
};

float plane_wave(float2 xz, PlaneWave wave, out float2 partial_xz)
{
    float2 dir = normalize(wave.dir);
    float k = 2 * 3.1416f / wave.wavelength;
    float2 k_vector = dir * k;
    float frequency = wave.speed * k;
    float phase = dot(k_vector, xz) + frequency * time + wave.phase0;
    float offset = wave.amplitude * sin(phase);
    partial_xz = wave.amplitude * cos(phase) * k_vector;
    return offset;
}

VSOutput main(VSInput input)
{
    // known mesh is a grid with 2x2 size
    const int pwaves_count = 3;
    PlaneWave pwaves[pwaves_count] = { float2(1, 3), 0.5f, 0.3f, 1, 0,
                                       float2(3, 1), 0.5f, 0.3f, 1, 0.3f ,
                                       float2(2, 2), 0.5f, 0.2f, 1, 0.6f };
    float offset = 0;
    float offset_max = 0;
    float2 partial_xz = { 0,0 };
    for (int i = 0; i < pwaves_count; i++)
    {
        float2 partial;
        offset += plane_wave(input.posL.xz, pwaves[i], partial);
        offset_max += pwaves[i].amplitude;
        partial_xz += partial;
    }

    float3 posL = input.posL;
    posL.y += offset / offset_max;
    partial_xz /= offset_max;
    float3 normL = { -partial_xz.x, 1,-partial_xz.y };
    float2 uv = input.uv + float2(1, 1) * 0.03f * time;

    VSOutput output;
    float4 pos = mul(float4(posL, 1.0f), world);
    output.posW = pos.xyz;
    output.posH = mul(pos, view_proj);
    float3x3 normal_world = { inv_world[0].xyz, inv_world[1].xyz, inv_world[2].xyz };
    output.normW = normalize(mul(normal_world, normL));
    output.color = input.color;
    output.uv = mul(float4(uv, 0, 1), uv_tf).xy;
    return output;
}
