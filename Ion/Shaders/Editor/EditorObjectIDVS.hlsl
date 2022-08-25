#include <ShaderCommon.hlsl>

Pixel VSMain(Vertex vertex)
{
	Pixel pixel;

	pixel.Location = mul(ModelViewProjectionMatrix, vertex.Location);
	pixel.Normal = vertex.Normal;
	pixel.TexCoord = vertex.TexCoord;

	pixel.LocationWS = (float4)0.0f;
	pixel.NormalWS = (float4)0.0f;

	return pixel;
}
