#include "IonPCH.h"

#include "Shader.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLShader.h"
#include "RHI/DX10/DX10Shader.h"
#include "RHI/DX11/DX11Shader.h"

namespace Ion
{
	TShared<RHIShader> RHIShader::Create()
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return MakeShared<OpenGLShader>();
		case ERHI::DX10:
			return MakeShared<DX10Shader>();
		case ERHI::DX11:
			return MakeShared<DX11Shader>();
		default:
			return TShared<RHIShader>(nullptr);
		}
	}
}
