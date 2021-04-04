#include "IonPCH.h"

#include "Shader.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLShader.h"

namespace Ion
{
	// ------------------------------------------
	// Shader 

	Shared<Shader> Shader::Create(EShaderType shaderType, std::string shaderSource)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLShader>(shaderType, shaderSource);
		default:
			return Shared<Shader>(nullptr);
		}
	}

	Shader::Shader(EShaderType shaderType, std::string shaderSource)
		: m_Type(shaderType), m_ShaderSource(shaderSource)
	{ }

	// ------------------------------------------
	// Program

	Shared<Program> Program::Create()
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLProgram>();
		default:
			return Shared<Program>(nullptr);
		}
	}
}
