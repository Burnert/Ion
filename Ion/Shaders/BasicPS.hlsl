cbuffer GlobalCBuffer
{
	float4 GColor;
};

float4 main(float4 position : SV_POSITION/*, float4 normal : NORMAL, float4 texcoord : TEXCOORD*/) : SV_TARGET
{
	return GColor;
}
