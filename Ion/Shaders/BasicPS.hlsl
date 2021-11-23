#include "ShaderCommon.hlsl"

Texture2D g_Texture;
SamplerState g_Sampler;

float4 main(Pixel pixel) : SV_TARGET
{
	return g_Texture.Sample(g_Sampler, pixel.TexCoord.xy);
}
