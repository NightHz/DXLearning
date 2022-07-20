#include "cbuffers.hlsli"

struct GSInput
{
	float3 posW : POSITION;
	float2 sizeW : SIZE;
};

struct GSOutput
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float2 uv : TEXCOORD;
	float3 normW : NORMAL;
};

[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> output)
{
	float3 at = eye_pos - input[0].posW;
	float3 up = float3(0, 1, 0);
	float3 right = normalize(cross(up, at));
	float2 half_size = input[0].sizeW / 2;

	GSOutput v;
	v.normW = normalize(at);

	v.posW = input[0].posW - half_size.x * right + half_size.y * up;
	v.posH = mul(float4(v.posW, 1), view_proj);
	v.uv = float2(0.05f, 0.05f);
	output.Append(v);

	v.posW = input[0].posW - half_size.x * right - half_size.y * up;
	v.posH = mul(float4(v.posW, 1), view_proj);
	v.uv = float2(0.05f, 0.95f);
	output.Append(v);

	v.posW = input[0].posW + half_size.x * right + half_size.y * up;
	v.posH = mul(float4(v.posW, 1), view_proj);
	v.uv = float2(0.95f, 0.05f);
	output.Append(v);

	v.posW = input[0].posW + half_size.x * right - half_size.y * up;
	v.posH = mul(float4(v.posW, 1), view_proj);
	v.uv = float2(0.95f, 0.95f);
	output.Append(v);
}
