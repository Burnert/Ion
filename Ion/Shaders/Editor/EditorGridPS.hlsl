#include "ShaderCommon.hlsl"

static const float LARGE_GRID_DENSITY = 1.0f;
static const float SMALL_GRID_DENSITY = 10.0f;

static const float LARGE_GRID_WIDTH = 0.004f;
static const float SMALL_GRID_WIDTH = 0.002f;

static const float3 X_AXIS_COLOR = float3(1.0f, 0.1f, 0.1f);
static const float3 Z_AXIS_COLOR = float3(0.2f, 0.2f, 1.0f);

static Pixel s_Pixel;

float OffsetWidthByDepth(float width)
{
	float depth = GetPixelDepth(s_Pixel.LocationWS.xyz);
	// Offset only by half
	width *= max(1.0f, depth * 0.5f);
	return width;
}

float2 GridUV(float2 uv, float density)
{
	// desmos:
	// \operatorname{abs}\left(\operatorname{mod}\left(\operatorname{abs}\left(x\right)+\frac{.5}{d},\frac{1}{d}\right)-\frac{.5}{d}\right)
	return abs(fmod(abs(uv) + (float2)(0.5f / density), 1.0f / density) - 0.5f / density);
}

bool IsOnGrid(float2 uv, float density, float gridWidth)
{
	float2 gridUv = GridUV(uv, density);
	return gridUv.x < gridWidth || gridUv.y < gridWidth;
}

float4 SampleGridColor(float2 uv)
{
	float4 grid = (float4)0.0f;

	if (IsOnGrid(uv, SMALL_GRID_DENSITY, OffsetWidthByDepth(SMALL_GRID_WIDTH)))
		grid = float4(0.6f, 0.6f, 0.6f, 0.1f);
	if (IsOnGrid(uv, LARGE_GRID_DENSITY, OffsetWidthByDepth(LARGE_GRID_WIDTH)))
		grid = float4(0.6f, 0.6f, 0.6f, 0.2f);

	float4 axisOverlay = 1.0f;
	axisOverlay = lerp(axisOverlay, float4(Z_AXIS_COLOR, 2.0f), saturate(1.0f - abs(uv.x))); // Z-Axis
	axisOverlay = lerp(axisOverlay, float4(X_AXIS_COLOR, 2.0f), saturate(1.0f - abs(uv.y))); // X-Axis
	grid *= axisOverlay;

	float depth = GetPixelDepth(s_Pixel.LocationWS.xyz);
	float depthMask = (100.0f - depth) / 100.0f;
	depthMask = pow(depthMask, 6);
	grid *= depthMask;

	return grid;
}

float4 PSMain(Pixel pixel) : SV_TARGET
{
	s_Pixel = pixel;

	float2 worldUV = pixel.TexCoord.xy;

	float4 color = SampleGridColor(worldUV);

	if (color.a == 0.0f)
		clip(-1);

	return float4(color.rgba);
}
