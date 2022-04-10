// Scene data

#define MAX_LIGHTS 100

struct Light
{
	float4 Location;
	float4 Color;
	float4 Direction;
	float Intensity;
	float Falloff;
};

// Constant Buffers

cbuffer SceneConstants : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float4x4 ViewProjectionMatrix;

	Light SceneLights[MAX_LIGHTS];
	Light SceneDirectionalLight;
	float4 SceneAmbientLightColor;

	float3 CameraLocation;

	uint SceneLightNum;
};

cbuffer MeshConstants : register(b1)
{
	float4x4 ModelViewProjectionMatrix;
	float4x4 TransformMatrix;
	float4x4 InverseTransposeMatrix;

	// With Editor

	uint4 RenderGuid;
};

// Input/Output Structs

struct Vertex
{
	float4 Location : POSITION;
	float4 Normal   : NORMAL;
	float4 TexCoord : TEXCOORD;
};

struct Pixel
{
	float4 Location : SV_POSITION;
	float4 Normal   : NORMAL;
	float4 TexCoord : TEXCOORD;

	float4 LocationWS : Pixel_LocationWS;
	float4 NormalWS   : Pixel_NormalWS;
};

struct BasicVertex
{
	float4 Location : POSITION;
	float4 TexCoord : TEXCOORD;
};

struct BasicPixel
{
	float4 Location : SV_POSITION;
	float4 TexCoord : TEXCOORD;
};
