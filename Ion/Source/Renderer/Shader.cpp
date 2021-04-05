#include "IonPCH.h"

#include "Shader.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLShader.h"

namespace Ion
{
	TShared<Shader> Shader::Create()
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLShader>();
		default:
			return TShared<Shader>(nullptr);
		}
	}
}
