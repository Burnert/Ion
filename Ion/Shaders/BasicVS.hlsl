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

struct Vertex
{
	float4 Position : POSITION;
	float4 Normal : NORMAL;
	float4 TexCoord : TEXCOORD;
};

struct Pixel
{
	float4 Position : SV_POSITION;
	float4 Normal : NORMAL;
	float4 TexCoord : TEXCOORD;
};

Pixel main(Vertex vertex)
{
	Pixel pixel;

	pixel.Position = mul(ViewProjectionMatrix, float4(vertex.Position.xyz, 1.0));
	pixel.Normal = vertex.Normal;
	pixel.TexCoord = vertex.TexCoord;

	return pixel;
}
