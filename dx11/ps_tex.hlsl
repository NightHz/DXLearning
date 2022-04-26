struct PSInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

Texture2D texture0 : register(t0);

SamplerState sampler0
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

float4 main(PSInput input) : SV_TARGET
{
	float4 color;
	color = texture0.Sample(sampler0, input.uv) * input.color;
	return color;
}
