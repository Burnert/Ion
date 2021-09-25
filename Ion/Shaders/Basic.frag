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

uniform sampler2D u_TextureSampler;
uniform vec3 u_CameraLocation;
uniform vec4 u_AmbientLightColor;
uniform DirectionalLight u_DirectionalLight;
uniform uint u_LightNum;
uniform Light u_Lights[MAX_LIGHTS];

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldNormal;
in vec3 v_PixelLocationWS;

out vec4 Color;

vec3 CalculateAmbientLight()
{
	return u_AmbientLightColor.xyz * u_AmbientLightColor.w;
}

vec3 CalculateDirectionalLight()
{
	float baseIntensity = max(dot(v_WorldNormal, -u_DirectionalLight.Direction), 0.0);
	return baseIntensity * u_DirectionalLight.Color * u_DirectionalLight.Intensity;
}

vec3 CalculateLight(Light light)
{
	vec3 inverseLightDir = normalize(light.Location - v_PixelLocationWS);
	float lightDistance = distance(light.Location, v_PixelLocationWS);
	float falloff = max((light.Falloff - lightDistance) / light.Falloff, 0.0);
	float lightIntensity = max(dot(inverseLightDir, v_WorldNormal), 0.0) * light.Intensity * falloff;
		
	return light.Color * lightIntensity;
}

void main()
{
	vec3 ambientLightColor = CalculateAmbientLight();
	vec3 dirLightColor = CalculateDirectionalLight();

	vec3 lightsColor = vec3(0.0);
	for (uint n = 0; n < u_LightNum; ++n)
	{
		Light light = u_Lights[n];
		lightsColor += CalculateLight(light);
	}

	// Adding lights together seems like a really weird thing to do here
	vec3 finalLightColor = ambientLightColor + dirLightColor + lightsColor;

	Color = texture(u_TextureSampler, v_TexCoord).rgba * vec4(finalLightColor, 1.0);

	// Visualize world normal
	// Color = vec4((v_WorldNormal + 1.0) * 0.5, 1.0);
}
