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
	bool RHI::Init(ERHI rhi, GenericWindow* window)
	{
		//SCOPED_PERFORMANCE_COUNTER(RenderAPI_InitTime);
		TRACE_FUNCTION();

		SetCurrent(rhi);

		switch (rhi)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERHI::OpenGL:
			{
				OpenGL::Init(window);
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:
			{
				DX11::Init(window);
				break;
			}
#endif
			default:
				return false;
		}
		return true;
	}

	void RHI::Shutdown()
	{
		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:
			{
				DX11::Shutdown();
				break;
			}
#endif
			default:
				return;
		}
	}

	void RHI::BeginFrame()
	{
		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_DX11
		case ERHI::DX11:
		{
			DX11::BeginFrame();
			break;
		}
#endif
		default:
			return;
		}
	}

	void RHI::EndFrame(GenericWindow& window)
	{
		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERHI::OpenGL:
			{
				OpenGL::EndFrame(window);
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:
			{
				DX11::EndFrame();
				break;
			}
#endif
			default:
				return;
		}
	}

	void RHI::ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height)
	{
		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_DX11
		case ERHI::DX11:
		{
			DX11::ChangeDisplayMode(window, mode, width, height);
			break;
		}
#endif
		default:
			return;
		}
	}

	void RHI::ResizeBuffers(GenericWindow& window, const TextureDimensions& size)
	{
		switch (s_CurrentRHI)
		{
		case ERHI::None:    break;
#if PLATFORM_SUPPORTS_DX11
		case ERHI::DX11:    DX11::ResizeBuffers(window, size); break;
#endif
		}
	}

	const char* RHI::GetCurrentDisplayName()
	{
		switch (s_CurrentRHI)
		{
		case ERHI::None:    return "Currently not using any Render API.";
#if PLATFORM_SUPPORTS_OPENGL
		case ERHI::OpenGL:  return OpenGL::GetDisplayName();
#endif
#if PLATFORM_SUPPORTS_DX11
		case ERHI::DX11:    return DX11::GetDisplayName();
#endif
		default:                  return "";
		}
	}

	void RHI::InitImGuiBackend()
	{
		TRACE_FUNCTION();

		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERHI::OpenGL:
			{
				OpenGL::InitImGuiBackend();
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:
			{
				DX11::InitImGuiBackend();
				break;
			}
#endif
		}
	}

	void RHI::ImGuiNewFrame()
	{
		TRACE_FUNCTION();

		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERHI::OpenGL:
			{
				OpenGL::ImGuiNewFrame();
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:
			{
				DX11::ImGuiNewFrame();
				break;
			}
#endif
		}
	}

	void RHI::ImGuiRender(ImDrawData* drawData)
	{
		TRACE_FUNCTION();

		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERHI::OpenGL:
			{
				OpenGL::ImGuiRender(drawData);
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:
			{
				DX11::ImGuiRender(drawData);
				break;
			}
#endif
		}
	}

	void RHI::ImGuiShutdown()
	{
		TRACE_FUNCTION();

		switch (s_CurrentRHI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERHI::OpenGL:
			{
				OpenGL::ImGuiShutdown();
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERHI::DX11:
			{
				DX11::ImGuiShutdown();
				break;
			}
#endif
		}
	}

	void RHI::SetCurrent(ERHI rhi)
	{
		s_CurrentRHI = rhi;
	}

	ERHI RHI::s_CurrentRHI = ERHI::None;
}
