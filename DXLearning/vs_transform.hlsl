struct Input
{
	vector pos : POSITION;
	vector color : COLOR;
};
struct Output
{
	vector pos : SV_POSITION;
	vector color : COLOR;
};

matrix transform;
vector color = { 1,1,1,1 };

Output main(Input input)
{
	Output output;
	output.pos = mul(input.pos, transform);
	output.color = color * input.color;
	return output;
}
