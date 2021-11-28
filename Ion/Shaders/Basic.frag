#version 430 core

#define MAX_LIGHTS 100
#define MAX_TEXTURES 16

struct Light
{
	vec4 Location;
	vec4 Color;
	vec4 Direction;
	float Intensity;
	float Falloff;
};

struct LightingData
{
	vec3 Diffuse;
	vec3 Specular;
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

//uniform vec3 u_CameraLocation;
//uniform vec3 u_CameraDirection;
//uniform vec4 u_AmbientLightColor;
//uniform DirectionalLight u_DirectionalLight;
//uniform uint u_LightNum;
//uniform Light u_Lights[MAX_LIGHTS];
uniform sampler2D g_Samplers[MAX_TEXTURES];

in vec2 v_TexCoord;
in vec3 v_WorldNormal;
in vec3 v_PixelLocationWS;

out vec4 Color;

// Normalize the world normal vector per pixel
vec3 WorldNormal = normalize(v_WorldNormal);

float SpecularIntensity = 0.5;

float CalculateSpecularIntensity(vec3 lightDirection, float shininess, float falloff = 1.0)
{
	// Points from the view towards the pixel in world space
	vec3 viewDirection = normalize(CameraLocation - v_PixelLocationWS);
	vec3 reflectedDirection = reflect(lightDirection, WorldNormal);
	return pow(max(dot(reflectedDirection, viewDirection), 0.0), shininess) * SpecularIntensity * falloff;
}

vec3 CalculateAmbientLight()
{
	return SceneAmbientLightColor.rgb * SceneAmbientLightColor.a;
}

LightingData CalculateDirectionalLight()
{
	LightingData data;

	float baseIntensity = max(dot(WorldNormal, -SceneDirectionalLight.Direction.xyz), 0.0);
	data.Diffuse = baseIntensity * SceneDirectionalLight.Color.rgb * SceneDirectionalLight.Intensity;

	float specular = CalculateSpecularIntensity(SceneDirectionalLight.Direction.xyz, 32.0) * SceneDirectionalLight.Intensity;
	data.Specular = SceneDirectionalLight.Color.rgb * specular;

	return data;
}

LightingData CalculateLight(Light light)
{
	LightingData data;

	vec3 lightDir = normalize(v_PixelLocationWS - light.Location.xyz);
	float lightDistance = distance(light.Location.xyz, v_PixelLocationWS);
	float falloff = max((light.Falloff - lightDistance) / light.Falloff, 0.0);
	float lightIntensity = max(dot(-lightDir, WorldNormal), 0.0) * light.Intensity * falloff;
	data.Diffuse = light.Color.rgb * lightIntensity;

	float specular = CalculateSpecularIntensity(lightDir, 32.0, falloff) * lightIntensity;
	data.Specular = light.Color.rgb * specular;

	return data;
}

LightingData CalculatePointLights()
{
	LightingData data;

	data.Diffuse = vec3(0.0);
	data.Specular = vec3(0.0);

	for (uint n = 0; n < SceneLightNum; ++n)
	{
		Light light = SceneLights[n];
		LightingData current = CalculateLight(light);

		data.Diffuse += current.Diffuse;
		data.Specular += current.Specular;
	}

	return data;
}

void main()
{
	vec3 ambientLightDiffuse = CalculateAmbientLight();

	LightingData directionalLight = CalculateDirectionalLight();

	LightingData pointLights = CalculatePointLights();

	vec3 finalLightDiffuse = ambientLightDiffuse + directionalLight.Diffuse + pointLights.Diffuse;
	vec3 finalLightSpecular = directionalLight.Specular + pointLights.Specular;

	Color = texture(g_Samplers[0], v_TexCoord).rgba * vec4(finalLightDiffuse, 1.0) + vec4(finalLightSpecular, 1.0);

	// Visualize world normal
	// Color = vec4((WorldNormal + 1.0) * 0.5, 1.0);

	// Color = vec4(v_PixelLocationWS, 1.0);
}
