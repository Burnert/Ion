struct VSOut
{
	float4 Position : SV_POSITION;
	//float4 Normal : NORMAL;
	//float4 TexCoord : TEXCOORD;
};

float4 main(float4 pos : POSITION/*, float4 normal : NORMAL, float4 texcoord : TEXCOORD*/) : SV_POSITION
{
	VSOut data;

	data.Position = pos;
	//data.Normal = normal;
	//data.TexCoord = texcoord;

	return pos;
}
