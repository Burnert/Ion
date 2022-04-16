#ifndef __FXAA_HLSL__
#define __FXAA_HLSL__

/**
 * FXAA algorithm implementation
 * Based on http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html
 */

#include "ShaderCommon.hlsl"

#define MIN_EDGE_THRESHOLD 0.0312f
#define MAX_EDGE_THRESHOLD 0.125f

#define EXPLORE_ITERATIONS 12

static const float EXPLORE_STEP[12] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 2.0f, 2.0f, 2.0f, 2.0f, 4.0f, 8.0f };

#define SUBPIXEL_QUALITY 0.75f

float4 ApplyFXAA(in Texture2D tex, in SamplerState smp, float2 uv)
{
	// Edge detection:

	// Get central values
	float4 colorCenter = tex.Sample(smp, uv);
	float lumaCenter = GetLuminosityRec709(colorCenter.rgb);

	// Get the neighbors' luminosity for edge detection
	float lumaUp    = GetLuminosityRec709(tex.Sample(smp, uv, uint2(0, -1)).rgb);
	float lumaDown  = GetLuminosityRec709(tex.Sample(smp, uv, uint2(0, 1)).rgb);
	float lumaLeft  = GetLuminosityRec709(tex.Sample(smp, uv, uint2(-1, 0)).rgb);
	float lumaRight = GetLuminosityRec709(tex.Sample(smp, uv, uint2(1, 0)).rgb);

	// Find minimum and maximum values
	float lumaMin = min(lumaCenter, min(lumaUp, min(lumaDown, min(lumaLeft, lumaRight))));
	float lumaMax = max(lumaCenter, max(lumaUp, max(lumaDown, max(lumaLeft, lumaRight))));

	float lumaRange = lumaMax - lumaMin;

	float4 finalColor = colorCenter;

	// Only perform the AA if there's an edge
	if (lumaRange >= max(MIN_EDGE_THRESHOLD, MAX_EDGE_THRESHOLD * lumaMax))
	{
		// Edge/gradient direction

		// Get the remaining neighbors
		float lumaUpLeft    = GetLuminosityRec709(tex.Sample(smp, uv, uint2(-1, -1)).rgb);
		float lumaDownLeft  = GetLuminosityRec709(tex.Sample(smp, uv, uint2(-1, 1)).rgb);
		float lumaUpRight   = GetLuminosityRec709(tex.Sample(smp, uv, uint2(1, -1)).rgb);
		float lumaDownRight = GetLuminosityRec709(tex.Sample(smp, uv, uint2(1, 1)).rgb);

		// Compute the sums of pixels for later use
		float lumaUpDown = lumaUp + lumaDown;
		float lumaLeftRight = lumaLeft + lumaRight;
		float lumaUpCorners = lumaUpLeft + lumaUpRight;
		float lumaDownCorners = lumaDownLeft + lumaDownRight;
		float lumaLeftCorners = lumaUpLeft + lumaDownLeft;
		float lumaRightCorners = lumaUpRight + lumaDownRight;

		// Estimate the gradient along the vertical and horizontal axis
		float edgeHorizontal =
			abs(-2.0f * lumaLeft + lumaLeftCorners) +
			abs(-2.0f * lumaCenter + lumaUpDown) * 2.0f +
			abs(-2.0f * lumaRight + lumaRightCorners);
		float edgeVertical =
			abs(-2.0f * lumaUp + lumaUpCorners) +
			abs(-2.0f * lumaCenter + lumaLeftRight) * 2.0f +
			abs(-2.0f * lumaDown + lumaDownCorners);

		bool bHorizontal = edgeHorizontal >= edgeVertical;

		// Find the edge:

		// Get the opposite luminosities
		float2 oppositeLums = bHorizontal ? float2(lumaUp, lumaDown) : float2(lumaLeft, lumaRight);
		float2 oppositeGradients = oppositeLums - lumaCenter;

		// Get the steeper gradient
		uint steeperGradient = (abs(oppositeGradients[0]) >= abs(oppositeGradients[1])) ? 0 : 1;

		float normalizedGradient = 0.25f * max(abs(oppositeGradients[0]), abs(oppositeGradients[1]));

		// Average luminosity in the direction of the steeper gradient (between the pixels)
		float lumaDirectionalAverage = 0.5f * (oppositeLums[steeperGradient] + lumaCenter);

		float2 inverseTextureSize = 1.0f / GetTextureDimensions(tex);

		// Shift the UV by half a pixel in the steeper gradient direction
		float shiftDirection = (steeperGradient == 0) ? -1.0f : 1.0f;
		// One pixel offset based on the shift direction and edge orientation
		float shiftBy = shiftDirection * (bHorizontal ? inverseTextureSize.y : inverseTextureSize.x);
		// Shift vertically by half a pixel if the edge is horizontal and vice-versa
		float2 shiftedUV = bHorizontal ? 
			float2(uv.x, uv.y + shiftBy * 0.5f) :
			float2(uv.x + shiftBy * 0.5f, uv.y);
		
		// Explore:

		// One pixel offset along the edge
		float2 exploreOffset = bHorizontal ? float2(inverseTextureSize.x, 0) : float2(0, inverseTextureSize.y);
		// Compute the offset UVs
		float2 exploreUVs[2] = { shiftedUV - exploreOffset, shiftedUV + exploreOffset };

		// Delta of the extreme samples to the "between the pixels" average
		float lumaExtremeDeltas[2] = {
			GetLuminosityRec709(tex.Sample(smp, exploreUVs[0]).rgb) - lumaDirectionalAverage,
			GetLuminosityRec709(tex.Sample(smp, exploreUVs[1]).rgb) - lumaDirectionalAverage,
		};

		bool bReachedEdge[2] = {
			abs(lumaExtremeDeltas[0]) >= normalizedGradient,
			abs(lumaExtremeDeltas[1]) >= normalizedGradient,
		};
		bool bReachedBoth = bReachedEdge[0] && bReachedEdge[1];

		if (!bReachedEdge[0])
			exploreUVs[0] -= exploreOffset;
		if (!bReachedEdge[1])
			exploreUVs[1] += exploreOffset;

		// Iterate:

		if (!bReachedBoth)
		{
			[unroll] for (int i = 2; i < EXPLORE_ITERATIONS; ++i)
			{
				// Explore further if the edge has not been reached yet
				if (!bReachedEdge[0])
					lumaExtremeDeltas[0] = GetLuminosityRec709(tex.Sample(smp, exploreUVs[0]).rgb) - lumaDirectionalAverage;
				if (!bReachedEdge[1])
					lumaExtremeDeltas[1] = GetLuminosityRec709(tex.Sample(smp, exploreUVs[1]).rgb) - lumaDirectionalAverage;

				// Update reached edge the same way
				bReachedEdge[0] = abs(lumaExtremeDeltas[0]) >= normalizedGradient;
				bReachedEdge[1] = abs(lumaExtremeDeltas[1]) >= normalizedGradient;
				bReachedBoth = bReachedEdge[0] && bReachedEdge[1];

				// Offset by an increasing amount of pixels
				if (!bReachedEdge[0])
					exploreUVs[0] -= exploreOffset * EXPLORE_STEP[i];
				if (!bReachedEdge[1])
					exploreUVs[1] += exploreOffset * EXPLORE_STEP[i];

				// If both edges have been reached, stop the iteration
				if (bReachedBoth)
					break;
			}
		}

		// Compute the distances from the starting pixel to the edges
		float edgeDistances[2] = {
			bHorizontal ? (uv.x - exploreUVs[0].x) : (uv.y - exploreUVs[0].y),
			bHorizontal ? (exploreUVs[1].x - uv.x) : (exploreUVs[1].y - uv.y),
		};
		// Get the closer edge
		uint closerEdge = (edgeDistances[0] < edgeDistances[1]) ? 0 : 1;
		float closerEdgeDistance = min(edgeDistances[0], edgeDistances[1]);

		float edgeLength = edgeDistances[0] + edgeDistances[1];

		// UV offset
		float pixelOffset = -closerEdgeDistance / edgeLength + 0.5f;

		bool bCenterDarkerThanAverage = lumaCenter < lumaDirectionalAverage;
		// If the center pixel is darker than the neighboring pixel, the edge delta should be positive
		bool bCorrectVariation = (lumaExtremeDeltas[closerEdge] < 0.0f) != bCenterDarkerThanAverage;
		// Don't actually offset the pixel, if the variation was incorrect
		float actualPixelOffset = bCorrectVariation ? pixelOffset : 0.0f;

		// Sub-pixel offset:

		// Weighted average of the 3x3 pixel area luminosity
		float lumaFullAverage = (2.0f * (lumaUpDown + lumaLeftRight) + lumaLeftCorners + lumaRightCorners) / 12.0f;

		float subPixelOffset1 = saturate(abs(lumaFullAverage - lumaCenter) / lumaRange);
		float subPixelOffset2 = (-2.0f * subPixelOffset1 + 3.0f) * subPixelOffset1 * subPixelOffset1;
		float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

		// Select the larger offset
		float finalOffset = max(actualPixelOffset, subPixelOffsetFinal);

		// Final UV

		float2 finalUV = bHorizontal ?
			float2(uv.x, uv.y + finalOffset * shiftBy) :
			float2(uv.x + finalOffset * shiftBy, uv.y);

		finalColor = tex.Sample(smp, finalUV);
	}

	return finalColor;
}

#endif // __FXAA_HLSL__
