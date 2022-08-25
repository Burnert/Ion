#include <ShaderCommon.hlsl>

Pixel VSMain(Vertex vertex)
{
	Pixel pixel;

	pixel.Location = mul(ModelViewProjectionMatrix, vertex.Location);
	pixel.Normal = vertex.Normal;
	pixel.TexCoord = float4(mul(TransformMatrix, vertex.Location).xz, 0.0f, 0.0f);

	pixel.LocationWS = mul(TransformMatrix, vertex.Location);
	pixel.NormalWS = (float4)0.0f;

	return pixel;
}
