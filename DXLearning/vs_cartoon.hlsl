struct VSInput
{
	float4 pos : POSITION;
	float4 color : COLOR;
	float4 normal : NORMAL;
	float2 tex : TEXCOORD;
};
struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};

matrix world_to_view_transform;
matrix obj_to_view_transform;
matrix obj_to_proj_transform;

float4 light_dir;
float4 diffuse = { 1,1,1,1 };

VSOutput main(VSInput input)
{
	VSOutput output;
	float4 pos = mul(input.pos, obj_to_proj_transform);
	output.pos = pos;

	float4 N = { input.normal.xyz, 0 };
	N = normalize(mul(N, obj_to_view_transform)); // normal
	float4 V = { -mul(input.pos, obj_to_view_transform).xyz, 0 };
	V = normalize(V); // to eye
	float4 ld = { light_dir.xyz, 0 };
	float4 L = normalize(-mul(ld, world_to_view_transform)); // to light

	float s = min(0.99f, max(0.01f, 0.9f * dot(L, N)));
	output.tex.x = s;
	output.tex.y = 0.5f;
	output.color = diffuse;

	return output;
}
