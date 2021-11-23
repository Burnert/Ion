// Scene data

#define MAX_LIGHTS 100

struct DirectionalLight
{
	float4 Direction;
	float4 Color;
	float Intensity;
};

struct Light
{
	float4 Location;
	float4 Color;
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
	DirectionalLight SceneDirectionalLight;

	float3 CameraLocation;
};

cbuffer MeshConstants : register(b1)
{
	float4x4 ModelViewProjectionMatrix;
	float4x4 TransformMatrix;
	float4x4 InverseTransposeMatrix;
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
