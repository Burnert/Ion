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

	float4x4 mvp = mul(ViewProjectionMatrix, TransformMatrix);
	pixel.Position = mul(ModelViewProjectionMatrix, vertex.Position);
	pixel.Normal = vertex.Normal;
	pixel.TexCoord = vertex.TexCoord;

	return pixel;
}
