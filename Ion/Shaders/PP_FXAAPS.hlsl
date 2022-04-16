#include "ShaderCommon.hlsl"
#include "FXAA.hlsl"

Texture2D g_Texture;
SamplerState g_Sampler;

float4 PSMain(BasicPixel pixel) : SV_TARGET
{
	return float4(ApplyFXAA(g_Texture, g_Sampler, pixel.TexCoord.xy).rgb, 1.0f);
}
