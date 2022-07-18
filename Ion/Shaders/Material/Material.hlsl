#ifndef __MATERIAL_HLSL__
#define __MATERIAL_HLSL__

#include "ShaderCommon.hlsl"

// Mark entry functions with this
#define ENTRY

Texture2D g_Texture[16];
SamplerState g_Sampler[16];

struct MaterialAttributesVS
{
	float3 WorldVertexOffset;
};

struct MaterialAttributesPS
{
	float3 BaseColor;
};

// Material functions Fwd Decl

MaterialAttributesPS MaterialMainPS(Pixel pixel);
MaterialAttributesVS MaterialMainVS(Vertex vertex);

#endif
