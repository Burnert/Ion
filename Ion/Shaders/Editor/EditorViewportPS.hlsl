#include "ShaderCommon.hlsl"

Texture2D g_Texture[2];
SamplerState g_Sampler[2];

float4 PSMain(BasicPixel pixel) : SV_TARGET
{
	float4 scene = float4(g_Texture[0].Sample(g_Sampler[0], pixel.TexCoord.xy).rgb, 1.0f);
	float editorDataDepth = g_Texture[1].Sample(g_Sampler[1], pixel.TexCoord.xy).r;

	if (editorDataDepth != 1.0f)
	{
		return saturate(lerp(scene, float4(0.0f, 0.5f, 1.0f, 1.0f), 0.5f));
	}
	else
	{
		return scene;
	}
}
