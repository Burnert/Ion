#include "BasicLit.hlsl"

cbuffer MaterialParameters : register(b2)
{
	float4 Param_Color;
	float Param_Brightness;
};

MaterialAttributesVS MaterialMainVS(Vertex vertex)
{
	MaterialAttributesVS attributes;

	return attributes;
}

MaterialAttributesPS MaterialMainPS(Pixel pixel)
{
	MaterialAttributesPS attributes;

	attributes.BaseColor = g_Texture[0].Sample(g_Sampler[0], pixel.TexCoord.xy).rgb * Param_Color.rgb * Param_Brightness;

	return attributes;
}
