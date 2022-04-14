#include "ShaderCommon.hlsl"

Texture2D g_AlphaMaskTexture;
SamplerState g_Sampler;

uint4 PSMain(Pixel pixel) : SV_TARGET
{
	float mask = g_AlphaMaskTexture.Sample(g_Sampler, pixel.TexCoord.xy).a;

	if (mask == 0.0f)
		clip(-1);

	return RenderGuid;
}
