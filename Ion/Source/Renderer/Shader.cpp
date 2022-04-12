#include "IonPCH.h"

#include "Shader.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLShader.h"
#include "RHI/DX11/DX11Shader.h"

namespace Ion
{
	TShared<Shader> Shader::Create()
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return MakeShared<OpenGLShader>();
		case ERHI::DX11:
			return MakeShared<DX11Shader>();
		default:
			return TShared<Shader>(nullptr);
		}
	}
}
