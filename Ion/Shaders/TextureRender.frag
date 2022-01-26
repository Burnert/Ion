#version 430 core

uniform sampler2D g_Sampler;

in vec2 v_TexCoord;

out vec4 Color;

void main()
{
	Color = texture(g_Sampler, v_TexCoord).rgba;
}
