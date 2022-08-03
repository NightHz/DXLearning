#include "cbuffers.hlsli"

struct HSOutput
{
	float3 posW : POSITION;
	float2 uv : TEXCOORD;
};

struct DSOutput
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float2 uv : TEXCOORD;
	float3 normW : NORMAL;
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

float4 BezierBase(float t)
{
	float k = 1 - t;
	return float4(k * k * k, 3 * k * k * t, 3 * k * t * t, t * t * t);
}

float4 BezierPartialBase(float t)
{
	float k = 1 - t;
	return float4(-3 * k * k, -6 * k * t + 3 * k * k, -3 * t * t + 6 * k * t, 3 * t * t);
}

float3 BezierSum(const OutputPatch<HSOutput, 16> patch, float4 b1, float4 b2)
{
	return b2.x * (b1.x * patch[0].posW + b1.y * patch[1].posW + b1.z * patch[2].posW + b1.w * patch[3].posW) +
		b2.y * (b1.x * patch[4].posW + b1.y * patch[5].posW + b1.z * patch[6].posW + b1.w * patch[7].posW) +
		b2.z * (b1.x * patch[8].posW + b1.y * patch[9].posW + b1.z * patch[10].posW + b1.w * patch[11].posW) +
		b2.w * (b1.x * patch[12].posW + b1.y * patch[13].posW + b1.z * patch[14].posW + b1.w * patch[15].posW);
}

[domain("quad")]
DSOutput main(HSConstantData input, float2 domain : SV_DomainLocation, const OutputPatch<HSOutput, 16> patch)
{
	float4 b1 = BezierBase(domain.x);
	float4 b2 = BezierBase(domain.y);
	float4 db1 = BezierPartialBase(domain.x);
	float4 db2 = BezierPartialBase(domain.y);

	float3 pos = BezierSum(patch, b1, b2);
	float3 t1 = BezierSum(patch, db1, b2);
	float3 t2 = BezierSum(patch, b1, db2);

	float2 uv = lerp(lerp(patch[0].uv, patch[3].uv, domain.x), lerp(patch[12].uv, patch[15].uv, domain.x), domain.y);
	float3 norm = cross(t1, t2);

	DSOutput output;
	output.posW = pos;
	output.posH = mul(float4(pos, 1), view_proj);
	output.normW = normalize(norm);
	output.uv = uv;
	return output;
}
