#include "IonPCH.h"

#include "RHI.h"

#if RHI_BUILD_OPENGL
#include "OpenGL/OpenGL.h"
#endif
#if RHI_BUILD_DX10
#include "DX10/DX10.h"
#endif
#if RHI_BUILD_DX11
#include "DX11/DX11.h"
#endif

//DECLARE_PERFORMANCE_COUNTER(RenderAPI_InitTime, "RenderAPI Init Time", "Init");

#define NOT_SUPPORTED_ERROR ionerror(RHIError, "{} RHI is not supported on this platform.", ERHIAsString(s_CurrentRHI));

namespace Ion
{
	RHI* RHI::Create(ERHI rhi)
	{
		s_CurrentRHI = rhi;

		switch (s_CurrentRHI)
		{
		case ERHI::OpenGL:
#if RHI_BUILD_OPENGL
			return s_RHI = new OpenGL;
#elif !PLATFORM_SUPPORTS_OPENGL
			NOT_SUPPORTED_ERROR
#endif
			break;

		case ERHI::DX10:
#if RHI_BUILD_DX10
			return s_RHI = new DX10;
#elif !PLATFORM_SUPPORTS_DX10
			NOT_SUPPORTED_ERROR
#endif
			break;

		case ERHI::DX11:
#if RHI_BUILD_DX11
			return s_RHI = new DX11;
#elif !PLATFORM_SUPPORTS_DX11
			NOT_SUPPORTED_ERROR
#endif
			break;

		default:
			s_CurrentRHI = ERHI::None;
		}

		ionerror(RHIError, "{} RHI has been disabled in the build config.", ERHIAsString(s_CurrentRHI));
		return nullptr;
	}

	void RHI::SetEngineShadersPath(const FilePath& path)
	{
		ionassert(s_EngineShadersPath.IsEmpty(), "Engine shaders path should be only set once.");
		s_EngineShadersPath = path;
	}

	ERHI RHI::s_CurrentRHI = ERHI::None;
	RHI* RHI::s_RHI = nullptr;

	FilePath RHI::s_EngineShadersPath;
}
