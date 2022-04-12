#include "IonPCH.h"

#include "RHI.h"

#if PLATFORM_SUPPORTS_OPENGL
#include "OpenGL/OpenGL.h"
#endif
#if PLATFORM_SUPPORTS_DX11
#include "DX11/DX11.h"
#endif

#include "Application/Window/GenericWindow.h"

//DECLARE_PERFORMANCE_COUNTER(RenderAPI_InitTime, "RenderAPI Init Time", "Init");

namespace Ion
{
	RHI* RHI::Create(ERHI rhi)
	{
		s_CurrentRHI = rhi;

		switch (rhi)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERHI::OpenGL: return s_RHI = new OpenGL;
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:   return s_RHI = new DX11;
#endif
		}
		s_CurrentRHI = ERHI::None;
		ionassert(0, "%s RHI not supported on this platform.", ERHIAsString(rhi).c_str());
		return nullptr;
	}

	ERHI RHI::s_CurrentRHI = ERHI::None;
	RHI* RHI::s_RHI = nullptr;
}
