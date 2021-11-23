#include "ShaderCommon.hlsl"

Pixel main(Vertex vertex)
{
	Pixel pixel;

	pixel.Location = mul(ModelViewProjectionMatrix, vertex.Location);
	pixel.Normal = vertex.Normal;
	pixel.TexCoord = vertex.TexCoord;

	pixel.LocationWS = mul(TransformMatrix, vertex.Location);
	pixel.NormalWS = mul(InverseTransposeMatrix, vertex.Normal);

	return pixel;
}
