#include "IonPCH.h"

#include "Shader.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLShader.h"
#include "RHI/DX10/DX10Shader.h"
#include "RHI/DX11/DX11Shader.h"

namespace Ion
{
	RHIShader::RHIShader()
	{
		RHILogger.Info("RHIShader \"TODO\" object has been created.");
	}

	RHIShader::~RHIShader()
	{
		RHILogger.Info("RHIShader \"TODO\" object has been destroyed.");
	}
}
