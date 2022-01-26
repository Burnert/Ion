#version 430 core

layout(location = 0) in vec3 a_Location;
layout(location = 1) in vec2 a_TexCoord0;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord0;
	gl_Position = vec4(a_Location, 1.0);
}
