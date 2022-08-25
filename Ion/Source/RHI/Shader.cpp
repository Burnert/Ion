#include "IonPCH.h"

#include "Shader.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLShader.h"
#include "RHI/DX10/DX10Shader.h"
#include "RHI/DX11/DX11Shader.h"

namespace Ion
{
	TRef<RHIShader> RHIShader::Create()
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return MakeRef<OpenGLShader>();
		case ERHI::DX10:
			return MakeRef<DX10Shader>();
		case ERHI::DX11:
			return MakeRef<DX11Shader>();
		default:
			return nullptr;
		}
	}
}
