Texture2D g_Texture;
SamplerState g_Sampler;

cbuffer SceneConstants : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float4x4 ViewProjectionMatrix;

	float3 CameraLocation;
};

cbuffer MeshConstants : register(b1)
{
	float4x4 ModelViewProjectionMatrix;
	float4x4 TransformMatrix;
	float4x4 InverseTransposeMatrix;
};

struct Pixel
{
	float4 Position : SV_POSITION;
	float4 Normal : NORMAL;
	float4 TexCoord : TEXCOORD;
};

struct PSOutData
{
	float4 Target : SV_TARGET;
};

PSOutData main(Pixel pixel)
{
	PSOutData data;

	data.Target = g_Texture.Sample(g_Sampler, pixel.TexCoord.xy);

	return data;
}
