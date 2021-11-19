#include "IonPCH.h"

#include "Shader.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLShader.h"
#include "RenderAPI/DX11/DX11Shader.h"

namespace Ion
{
	TShared<Shader> Shader::Create()
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLShader>();
		case ERenderAPI::DX11:
			return MakeShared<DX11Shader>();
		default:
			return TShared<Shader>(nullptr);
		}
	}
}
