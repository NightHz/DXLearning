struct Input
{
	vector pos : POSITION;
	vector color : COLOR;
	float2 tex : TEXCOORD;
};
struct Output
{
	vector pos : SV_POSITION;
	vector color : COLOR;
	float2 tex : TEXCOORD;
};

matrix transform;

float time;

Output main(Input input)
{
	Output output;
	output.pos = mul(input.pos, transform);
	float t = time % 6.0f;
	float4 color = { 0,0,0,1 };
	if (t < 0.5f) { color.r = 1; }
	else if (t < 1.5f) { color.r = 1.5f - t; color.g = t - 0.5f; }
	else if (t < 2.0f) { color.g = 1; }
	else if (t < 3.0f) { color.g = 3.0f - t; color.b = t - 2.0f; }
	else if (t < 3.5f) { color.b = 1; }
	else if (t < 4.5f) { color.b = 1; color.rg = t - 3.5f; }
	else if (t < 5.0f) { color.rgb = 1; }
	else if (t < 6.0f) { color.gb = 6.0f - t; color.r = 1; }
	output.color = color * input.color;
	output.tex = input.tex;
	return output;
}
