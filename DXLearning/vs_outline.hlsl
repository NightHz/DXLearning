struct VSInput
{
	float4 pos : POSITION;
	float4 normal : NORMAL;
};
struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

matrix obj_to_proj_transform;

float outline = 0.05f;

float4 diffuse = { 0,0,0,1 };
float4 black = { 0,0,0,1 };
float4 white = { 1,1,1,1 };

VSOutput main(VSInput input)
{
	VSOutput output;

	input.normal.w = 0;
	input.pos += outline * input.normal;
	output.pos = mul(input.pos, obj_to_proj_transform);
	output.color = black;

	return output;
}
