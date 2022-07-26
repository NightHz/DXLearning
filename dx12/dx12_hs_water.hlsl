#include "cbuffers.hlsli"

struct VSOutput
{
	float3 posW : POSITION;
	float2 uv : TEXCOORD;
	float3 normW : NORMAL;
};

struct HSOutput
{
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

HSConstantData HSCalcConstantData(InputPatch<VSOutput, 4> patch)
{
	float3 center = (patch[0].posW + patch[1].posW + patch[2].posW + patch[3].posW) / 4;
	float dis = length(center - eye_pos);
	const float d0 = 5;
	const float d1 = 40;
	float tess = 60 * saturate((d1 - dis) / (d1 - d0)) + 4;

	HSConstantData output;
	output.tess.edge[0] = tess;
	output.tess.edge[1] = tess;
	output.tess.edge[2] = tess;
	output.tess.edge[3] = tess;
	output.tess.inside[0] = tess;
	output.tess.inside[1] = tess;
	return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSCalcConstantData")]
HSOutput main(InputPatch<VSOutput, 4> patch, uint i : SV_OutputControlPointID)
{
	HSOutput output;
	output.posW = patch[i].posW;
	output.uv = patch[i].uv;
	output.normW = patch[i].normW;
	return output;
}
