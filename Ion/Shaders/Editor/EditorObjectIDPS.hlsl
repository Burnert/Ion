#include "ShaderCommon.hlsl"

uint4 PSMain(Pixel pixel) : SV_TARGET
{
	return RenderGuid;
}
