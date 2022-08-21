#include "IonPCH.h"

#include "Shader.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLShader.h"
#include "RHI/DX10/DX10Shader.h"
#include "RHI/DX11/DX11Shader.h"

namespace Ion
{
	std::shared_ptr<RHIShader> RHIShader::Create()
	{
		switch (RHI::GetCurrent())
		{
		case ERHI::OpenGL:
			return std::make_shared<OpenGLShader>();
		case ERHI::DX10:
			return std::make_shared<DX10Shader>();
		case ERHI::DX11:
			return std::make_shared<DX11Shader>();
		default:
			return std::shared_ptr<RHIShader>(nullptr);
		}
	}
}
