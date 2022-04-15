#include "ShaderCommon.hlsl"

static const float LARGE_GRID_DENSITY = 1.0f;
static const float SMALL_GRID_DENSITY = 10.0f;

static const float LARGE_GRID_WIDTH = 0.002f;
static const float SMALL_GRID_WIDTH = 0.001f;
static const float AXIS_WIDTH = 0.003f;

static const float3 X_AXIS_COLOR = float3(1.0f, 0.1f, 0.1f);
static const float3 Z_AXIS_COLOR = float3(0.2f, 0.2f, 1.0f);

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
	float4 color = (float4)0.0f;

	if (IsOnGrid(uv, SMALL_GRID_DENSITY, SMALL_GRID_WIDTH))
		color += float4(1.0f, 1.0f, 1.0f, 0.2f);
	if (IsOnGrid(uv, LARGE_GRID_DENSITY, LARGE_GRID_WIDTH))
		color += float4(1.0f, 1.0f, 1.0f, 0.3f);

	if (abs(uv.x) < AXIS_WIDTH) // Z-Axis
		color = float4(Z_AXIS_COLOR, 0.5f);
	if (abs(uv.y) < AXIS_WIDTH) // X-Axis
		color = float4(X_AXIS_COLOR, 0.5f);

	return color;
}

float4 PSMain(Pixel pixel) : SV_TARGET
{
	float2 worldUV = pixel.TexCoord.xy;

	float4 color = SampleGridColor(worldUV);

	if (color.a == 0.0f)
		clip(-1);

	return float4(color.rgba);
}
