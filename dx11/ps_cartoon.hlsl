struct PSInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
	float4 colors[3] = { float4(0.4353f,0.3804f,0.3020f,1),float4(0.7804f,0.6706f,0.4627f,1),float4(0.9686f,0.8627f,0.5843f,1) };
	float ss[2] = { 0.26f,0.52f };
	for (int i = 0; i < 2; i++)
	{
		if (input.uv.x < ss[i])
			return colors[i];
	}
	return colors[2];
}
