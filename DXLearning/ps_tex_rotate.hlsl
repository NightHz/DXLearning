struct PSInput
{
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};
struct PSOutput
{
	float4 color : SV_TARGET;
};

sampler2D tex1;
float time;

/*texture2D tex2;
SamplerState tex2ss
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};*/

PSOutput main(PSInput input)
{
	PSOutput output;

	float t = time % 7.0f;
	float2 uv = input.uv;
	float4 color = { 0,0,0,1 };
	if (t < 1.0f) {}
	else if (t < 2.0f) { float t2 = t - 1.0f; uv = uv * (-t2 * (2 / 3.0f) + 1) + t2; }
	else if (t < 2.5f) { uv = uv * (1 / 3.0f); }
	else if (t < 5.5f)
	{
		uv = uv * (1 / 3.0f);
		float t2 = (t - 2.5f) / 3;
		float theta = t2 * 2 * 3.1416f;
		float ct = cos(theta);
		float st = sin(theta);
		float2x2 rotate = { ct,-st,st,ct };
		uv = mul(rotate, uv);
	}
	else if (t < 6.0f) { uv = uv * (1 / 3.0f); }
	else if (t < 7.0f) { float t2 = t - 6.0f; uv = uv * (t2 * (2 / 3.0f) + (1 / 3.0f)) + (1 - t2); }

	output.color = tex2D(tex1, uv); // for dx9 : use shader model 3
	//output.color = tex2.Sample(tex2ss, uv); // for dx11

	return output;
}
