#include "ShaderCommon.hlsl"

Texture2D g_Texture[3];
SamplerState g_Sampler[3];

static const float OutlineDisplaceUV[] = {
	{  0.0f,   -1.0f   },
	{  0.707f, -0.707f },
	{  1.0f,    0.0f   },
	{  0.707f,  0.707f },
	{  0.0f,    0.0f   },
	{ -0.707f, -0.707f },
	{ -1.0f,   -1.0f   },
	{ -0.707f, -0.707f },
};
static const float2 OutlineWidth = float2(0.001f, 0.002f);

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
	float4 scene = float4(g_Texture[0].Sample(g_Sampler[0], pixel.TexCoord.xy).rgb, 1.0f);
	float sceneDepth = g_Texture[1].Sample(g_Sampler[1], pixel.TexCoord.xy).r;
	float editorDataDepth = g_Texture[2].Sample(g_Sampler[2], pixel.TexCoord.xy).r;

	float outline = MakeOutline(pixel, g_Texture[2], g_Sampler[2]);

	float4 finalColor;

	if (editorDataDepth != 1.0f)
	{
		bool bIsInFront = editorDataDepth <= sceneDepth;
		finalColor = saturate(lerp(scene, SelectionColor, bIsInFront ? 0.5f : 0.2f));
	}
	else
	{
		finalColor = scene;
	}

	finalColor += outline * SelectionColor;
	return finalColor;
}
