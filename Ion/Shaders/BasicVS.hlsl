struct VSOut
{
	float4 Position : SV_POSITION;
	//float4 Normal : NORMAL;
	//float4 TexCoord : TEXCOORD;
};

cbuffer GlobalCBuffer : register(b0)
{
	float4 GColor;
};

cbuffer SceneConstants : register(b1)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float4x4 ViewProjectionMatrix;

	float3 CameraLocation;
};

cbuffer MeshConstants : register(b2)
{
	float4x4 ModelViewProjectionMatrix;
	float4x4 TransformMatrix;
	float4x4 InverseTransposeMatrix;
};

float4 main(float4 pos : POSITION/*, float4 normal : NORMAL, float4 texcoord : TEXCOORD*/) : SV_POSITION
{
	VSOut data;

	data.Position = mul(ViewProjectionMatrix, float4(pos.xyz, 1.0));
	//data.Normal = normal;
	//data.TexCoord = texcoord;

	return data.Position;
}
