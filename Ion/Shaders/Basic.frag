#version 430 core

struct DirectionalLight
{
	vec3 Direction;
	vec3 Color;
	float Intensity;
};

struct Light
{
	vec3 Location;
	vec3 Color;
	float Intensity;
	float Falloff;
};

#define MAX_LIGHTS 100
#define MAX_TEXTURES 16

uniform vec3 u_CameraLocation;
uniform vec3 u_CameraDirection;
uniform vec4 u_AmbientLightColor;
uniform DirectionalLight u_DirectionalLight;
uniform uint u_LightNum;
uniform Light u_Lights[MAX_LIGHTS];
uniform sampler2D u_Samplers[MAX_TEXTURES];

in vec2 v_TexCoord;
in vec3 v_WorldNormal;
in vec3 v_PixelLocationWS;

out vec4 Color;

// Normalize the world normal vector per pixel
vec3 WorldNormal = normalize(v_WorldNormal);

float SpecularIntensity = 0.5;

float CalculateSpecularIntensity(vec3 lightDirection, float shininess)
{
	// Points from the view towards the pixel in world space
	vec3 viewDirection = normalize(u_CameraLocation - v_PixelLocationWS);
	vec3 reflectedDirection = reflect(lightDirection, WorldNormal);
	return pow(max(dot(reflectedDirection, viewDirection), 0.0), shininess) * SpecularIntensity;
}

vec3 CalculateAmbientLight()
{
	return u_AmbientLightColor.xyz * u_AmbientLightColor.w;
}

void CalculateDirectionalLight(out vec3 outDiffuse, out vec3 outSpecular)
{
	float baseIntensity = max(dot(WorldNormal, -u_DirectionalLight.Direction), 0.0);
	outDiffuse = baseIntensity * u_DirectionalLight.Color * u_DirectionalLight.Intensity;

	float specular = CalculateSpecularIntensity(u_DirectionalLight.Direction, 32);
	outSpecular = u_DirectionalLight.Color * specular;
}

void CalculateLight(Light light, out vec3 outDiffuse, out vec3 outSpecular)
{
	vec3 lightDir = normalize(v_PixelLocationWS - light.Location);
	float lightDistance = distance(light.Location, v_PixelLocationWS);
	float falloff = max((light.Falloff - lightDistance) / light.Falloff, 0.0);
	float lightIntensity = max(dot(-lightDir, WorldNormal), 0.0) * light.Intensity * falloff;
	outDiffuse = light.Color * lightIntensity;

	float specular = CalculateSpecularIntensity(lightDir, 32);
	outSpecular = light.Color * specular;
}

void CalculatePointLights(out vec3 outDiffuse, out vec3 outSpecular)
{
	vec3 lightsDiffuse = vec3(0.0);
	vec3 lightsSpecular = vec3(0.0);
	for (uint n = 0; n < u_LightNum; ++n)
	{
		Light light = u_Lights[n];
		vec3 diffuse, specular;
		CalculateLight(light, diffuse, specular);
		lightsDiffuse += diffuse;
		lightsSpecular += specular;
	}

	outDiffuse = lightsDiffuse;
	outSpecular = lightsSpecular;
}

void main()
{
	vec3 ambientLightDiffuse = CalculateAmbientLight();

	vec3 dirLightDiffuse, dirLightSpecular;
	CalculateDirectionalLight(dirLightDiffuse, dirLightSpecular);

	vec3 pointLightsDiffuse, pointLightsSpecular;
	CalculatePointLights(pointLightsDiffuse, pointLightsSpecular);

	// I guess that's fine
	vec3 finalLightDiffuse = ambientLightDiffuse + dirLightDiffuse + pointLightsDiffuse;
	vec3 finalLightSpecular = dirLightSpecular + pointLightsSpecular;

	Color = texture(u_Samplers[0], v_TexCoord).rgba * vec4(finalLightDiffuse, 1.0) + vec4(finalLightSpecular, 1.0);

	// Visualize world normal
	// Color = vec4((WorldNormal + 1.0) * 0.5, 1.0);

	// Color = vec4(v_PixelLocationWS, 1.0);
}
