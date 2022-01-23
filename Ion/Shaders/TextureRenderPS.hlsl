#include "ShaderCommon.hlsl"

Texture2D g_Texture;
SamplerState g_Sampler;

float4 PSMain(BasicPixel pixel) : SV_TARGET
{
	return float4(g_Texture.Sample(g_Sampler, pixel.TexCoord.xy).rgb, 1.0f);
}
