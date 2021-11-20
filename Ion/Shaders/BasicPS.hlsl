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

float4 main(float4 position : SV_POSITION/*, float4 normal : NORMAL, float4 texcoord : TEXCOORD*/) : SV_TARGET
{
	return GColor;
}
