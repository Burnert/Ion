#include "ShaderCommon.hlsl"

Texture2D g_Texture;
SamplerState g_Sampler;

float4 PSMain(Pixel pixel) : SV_TARGET
{
	float4 color = g_Texture.Sample(g_Sampler, pixel.TexCoord.xy).rgba;

	if (color.a < 0.5f)
		clip(-1);

	return float4(color.rgb, 1.0f);
}
