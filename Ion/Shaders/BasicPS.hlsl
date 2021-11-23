#include "ShaderCommon.hlsl"

Texture2D g_Texture;
SamplerState g_Sampler;

static float4 s_LocationWS;
static float4 s_NormalWS;

struct LightingData
{
	float3 Diffuse;
	float3 Specular;
};

float g_SpecularIntensity = 0.5f;

float CalculateSpecularIntensity(float3 lightDirection, float shininess, float falloff = 1.0f)
{
	// Points from the view towards the pixel in world space
	float3 viewDirection = normalize((CameraLocation - s_LocationWS.xyz));
	float3 reflectedDirection = reflect(lightDirection, s_NormalWS.xyz);
	return pow(max(dot(reflectedDirection, viewDirection), 0.0f), shininess) * g_SpecularIntensity * falloff;
}

float3 CalculateAmbientLight()
{
	return SceneAmbientLightColor.rgb * SceneAmbientLightColor.a;
}

LightingData CalculateDirectionalLight()
{
	LightingData data;

	float baseIntensity = max(dot(s_NormalWS.xyz, -SceneDirectionalLight.Direction.xyz), 0.0f);
	data.Diffuse = SceneDirectionalLight.Color.rgb * SceneDirectionalLight.Intensity * baseIntensity;

	float specularIntensity = CalculateSpecularIntensity(SceneDirectionalLight.Direction.xyz, 32.0f) * SceneDirectionalLight.Intensity;
	data.Specular = SceneDirectionalLight.Color.rgb * specularIntensity;

	return data;
}

LightingData CalculateLight(Light light)
{
	LightingData data;

	float3 lightDir = normalize(s_LocationWS.xyz - light.Location.xyz);
	float lightDistance = distance(light.Location.xyz, s_LocationWS.xyz);
	float falloff = max((light.Falloff - lightDistance) / light.Falloff, 0.0f);
	float lightIntensity = max(dot(-lightDir, s_NormalWS.xyz), 0.0f) * light.Intensity * falloff;
	data.Diffuse = light.Color.rgb * lightIntensity;

	float specular = CalculateSpecularIntensity(lightDir, 32.0f, falloff) * lightIntensity;
	data.Specular = light.Color.rgb * specular;

	return data;
}

LightingData CalculateScenePointLights()
{
	LightingData data;

	data.Diffuse = 0.0f;
	data.Specular = 0.0f;

	for (uint n = 0; n < SceneLightNum; ++n)
	{
		Light light = SceneLights[n];
		LightingData current = CalculateLight(light);
		
		data.Diffuse += current.Diffuse;
		data.Specular += current.Specular;
	}

	return data;
}

float4 PSMain(Pixel pixel) : SV_TARGET
{
	s_LocationWS = pixel.LocationWS;
	s_NormalWS = float4(normalize(pixel.NormalWS.xyz), 0.0f);

	float3 ambientLightDiffuse = CalculateAmbientLight();
	LightingData dirLightData = CalculateDirectionalLight();
	LightingData pointLightsData = CalculateScenePointLights();

	float3 finalLightDiffuse = ambientLightDiffuse + dirLightData.Diffuse + pointLightsData.Diffuse;
	float3 finalLightSpecular = dirLightData.Specular + pointLightsData.Specular;

	return float4(g_Texture.Sample(g_Sampler, pixel.TexCoord.xy).rgb * finalLightDiffuse + finalLightSpecular, 1.0f);
}
