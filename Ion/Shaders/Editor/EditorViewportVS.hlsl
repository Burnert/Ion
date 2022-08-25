#include <ShaderCommon.hlsl>

BasicPixel VSMain(BasicVertex vertex)
{
	BasicPixel pixel;

	pixel.Location = vertex.Location;
	pixel.TexCoord = vertex.TexCoord;

	return pixel;
}
