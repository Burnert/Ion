#include "IonPCH.h"

#include "Shader.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLShader.h"
#include "RHI/DX11/DX11Shader.h"

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
