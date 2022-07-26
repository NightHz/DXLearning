#include "cbuffers.hlsli"
#include "water_wave.hlsli"

struct HSOutput
{
	float3 posW : POSITION;
	float2 uv : TEXCOORD;
	float3 normW : NORMAL;
};

struct DSOutput
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normW : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct QuadTess
{
	float edge[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct HSConstantData
{
	QuadTess tess;
};

[domain("quad")]
DSOutput main(HSConstantData input, float2 domain : SV_DomainLocation, const OutputPatch<HSOutput, 4> patch)
{
	float4 pos = float4(lerp(lerp(patch[0].posW, patch[1].posW, domain.x), lerp(patch[2].posW, patch[3].posW, domain.x), domain.y), 1);
	float2 uv = lerp(lerp(patch[0].uv, patch[1].uv, domain.x), lerp(patch[2].uv, patch[3].uv, domain.x), domain.y);
	float3 norm = lerp(lerp(patch[0].normW, patch[1].normW, domain.x), lerp(patch[2].normW, patch[3].normW, domain.x), domain.y);

	float2 partial_xz;
	float offset = get_water_offset(pos.xz, time, partial_xz);
	float4 up = mul(float4(0, 1, 0, 0), world);
	pos += up * offset;
	norm += float3(-partial_xz.x, 0, -partial_xz.y);

	DSOutput output;
	output.posW = pos.xyz;
	output.posH = mul(pos, view_proj);
	output.normW = normalize(norm);
	output.color = float4(1, 1, 1, 1);
	output.uv = uv;
	return output;
}
