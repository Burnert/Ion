#version 430 core

layout(location = 0) in vec3 a_Location;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord0;

uniform mat4 u_MVP;
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_InverseTranspose;
uniform vec3 u_CameraLocation;

out vec2 v_TexCoord;
out vec3 v_WorldNormal;
out vec3 v_PixelLocationWS;

void main()
{
	vec4 normal = vec4(a_Normal, 0.0);
	vec4 location = vec4(a_Location, 1.0);

	v_TexCoord = a_TexCoord0;
	v_WorldNormal = vec3(u_InverseTranspose * normal);
	v_PixelLocationWS = vec3(u_ModelMatrix * location);

	gl_Position = u_MVP * location;
}