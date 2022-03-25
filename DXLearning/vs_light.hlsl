struct VSInput
{
	float4 pos : POSITION;
	float4 color : COLOR;
	float4 normal : NORMAL;
	float4 tex : TEXCOORD;
};
struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

float one = 1;

matrix world_to_view_transform;
matrix obj_to_view_transform;
matrix obj_to_proj_transform;

bool light_enable = true;
bool specular_enable = false;
float4 light_dir;
float4 ambient = { 0.3f,0.3f,0.3f,1 };
float4 diffuse = { 0.984f,0.867f,0.573f,1 };
float4 specular = { 0.6f,0.6f,0.6f,1 };
float specular_a = 4;

float time = 0;
float delta_time = 1;

VSOutput main(VSInput input)
{
	// not used parameters , avoid delete
	float one2 = one;
	if (time == -234) one2 = 1;
	if (delta_time == -234) one2 = 1;

	VSOutput output;
	float4 pos = mul(input.pos, obj_to_proj_transform);
	output.pos = pos;
	if (light_enable)
	{
		float4 N = { input.normal.xyz, 0 };
		N = normalize(mul(N, obj_to_view_transform)); // normal
		float4 V = { -mul(input.pos, obj_to_view_transform).xyz, 0 };
		V = normalize(V); // to eye
		float4 ld = { light_dir.xyz, 0 };
		float4 L = normalize(-mul(ld, world_to_view_transform)); // to light
		float4 R = normalize(reflect(-L, N)); // reflect

		float4 c = ambient;
		float s = dot(L, N);
		if (s > 0)
		{
			c += diffuse * s;
			if (specular_enable)
				c += specular * pow(max(0, dot(R, V)), specular_a);
		}
		output.color = c;
	}
	else
		output.color = one2 * diffuse * input.color;
	return output;
}
