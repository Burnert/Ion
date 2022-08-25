#include <ShaderCommon.hlsl>

Texture2D g_SceneTexture : register(t0);
Texture2D g_SceneDepthTexture : register(t1);
Texture2D g_SelectionTexture : register(t2);

SamplerState g_SceneSampler[2] : register(s0);
SamplerState g_SelectionSampler : register(s2);

static const float2 OutlineDisplaceUV[] = {
	{  0.0f,   -1.0f   },
	{  0.707f, -0.707f },
	{  1.0f,    0.0f   },
	{  0.707f,  0.707f },
	{  0.0f,    1.0f   },
	{ -0.707f,  0.707f },
	{ -1.0f,    0.0f   },
	{ -0.707f, -0.707f },
};
static const float2 OutlineWidth = float2(0.002f, 0.004f);

static const float4 SelectionColor = float4(0.0f, 0.5f, 1.0f, 1.0f);

float MakeOutline(BasicPixel pixel, Texture2D tex, SamplerState smp)
{
	// @TODO: Use screen resolution to make it uniform

	float shape = 1.0f - floor(tex.Sample(smp, pixel.TexCoord.xy).r);
	float extendedShape = 0.0f;
	for (int i = 0; i < 8; ++i)
	{
		float2 displaceUV = OutlineDisplaceUV[i];
		float displacedShape = tex.Sample(smp, pixel.TexCoord.xy + OutlineWidth * displaceUV).r;
		displacedShape = displacedShape == 1.0f ? 0.0f : displacedShape;
		extendedShape = max(extendedShape, displacedShape);
	}
	extendedShape = ceil(extendedShape);
	return saturate(extendedShape - shape);
}

float4 PSMain(BasicPixel pixel) : SV_TARGET
{
	float4 scene = float4(g_SceneTexture.Sample(g_SceneSampler[0], pixel.TexCoord.xy).rgb, 1.0f);
	float sceneDepth = g_SceneDepthTexture.Sample(g_SceneSampler[1], pixel.TexCoord.xy).r;
	float selectionDepth = g_SelectionTexture.Sample(g_SelectionSampler, pixel.TexCoord.xy).r;

	float outline = MakeOutline(pixel, g_SelectionTexture, g_SelectionSampler);

	float4 finalColor;

	if (selectionDepth != 1.0f)
	{
		bool bIsInFront = selectionDepth <= sceneDepth;
		finalColor = saturate(lerp(scene, SelectionColor, bIsInFront ? 0.5f : 0.2f));
	}
	else
	{
		finalColor = scene;
	}

	finalColor += outline * SelectionColor;
	return finalColor;
}
