#version 430 core

#define MAX_LIGHTS 100

layout(location = 0) in vec3 a_Location;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord0;

struct Light
{
	vec4 Location;
	vec4 Color;
	vec4 Direction;
	float Intensity;
	float Falloff;
};

layout (std140, binding = 0) uniform SceneConstants
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	mat4 ViewProjectionMatrix;

	Light SceneLights[MAX_LIGHTS];
	Light SceneDirectionalLight;
	vec4 SceneAmbientLightColor;

	vec3 CameraLocation;

	uint SceneLightNum;
};

layout (std140, binding = 1) uniform MeshConstants
{
	mat4 ModelViewProjectionMatrix;
	mat4 TransformMatrix;
	mat4 InverseTransposeMatrix;
};

//uniform mat4 u_MVP;
//uniform mat4 u_ModelMatrix;
//uniform mat4 u_ViewMatrix;
//uniform mat4 u_ProjectionMatrix;
//uniform mat4 u_ViewProjectionMatrix;
//uniform mat4 u_InverseTranspose;
//uniform vec3 u_CameraLocation;

out vec2 v_TexCoord;
out vec3 v_WorldNormal;
out vec3 v_PixelLocationWS;

void main()
{
	vec4 normal = vec4(a_Normal, 0.0);
	vec4 location = vec4(a_Location, 1.0);

	v_TexCoord = a_TexCoord0;
	v_WorldNormal = vec3(InverseTransposeMatrix * normal);
	v_PixelLocationWS = vec3(TransformMatrix * location);

	gl_Position = ModelViewProjectionMatrix * location;
}
