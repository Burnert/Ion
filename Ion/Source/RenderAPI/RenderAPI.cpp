#include "IonPCH.h"

#include "RenderAPI.h"

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
	bool RenderAPI::Init(ERenderAPI api, GenericWindow* window)
	{
		//SCOPED_PERFORMANCE_COUNTER(RenderAPI_InitTime);
		TRACE_FUNCTION();

		SetCurrent(api);

		switch (api)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERenderAPI::OpenGL:
			{
				OpenGL::Init(window);
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERenderAPI::DX11:
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

	void RenderAPI::Shutdown()
	{
		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_DX11
			case ERenderAPI::DX11:
			{
				DX11::Shutdown();
				break;
			}
#endif
			default:
				return;
		}
	}

	void RenderAPI::BeginFrame()
	{
		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_DX11
		case ERenderAPI::DX11:
		{
			DX11::BeginFrame();
			break;
		}
#endif
		default:
			return;
		}
	}

	void RenderAPI::EndFrame(GenericWindow& window)
	{
		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERenderAPI::OpenGL:
			{
				OpenGL::EndFrame(window);
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERenderAPI::DX11:
			{
				DX11::EndFrame();
				break;
			}
#endif
			default:
				return;
		}
	}

	void RenderAPI::ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height)
	{
		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_DX11
		case ERenderAPI::DX11:
		{
			DX11::ChangeDisplayMode(window, mode, width, height);
			break;
		}
#endif
		default:
			return;
		}
	}

	void RenderAPI::ResizeBuffers(GenericWindow& window, const TextureDimensions& size)
	{
		switch (s_CurrentRenderAPI)
		{
		case ERenderAPI::None:    break;
#if PLATFORM_SUPPORTS_DX11
		case ERenderAPI::DX11:    DX11::ResizeBuffers(window, size); break;
#endif
		}
	}

	const char* RenderAPI::GetCurrentDisplayName()
	{
		switch (s_CurrentRenderAPI)
		{
		case ERenderAPI::None:    return "Currently not using any Render API.";
#if PLATFORM_SUPPORTS_OPENGL
		case ERenderAPI::OpenGL:  return OpenGL::GetDisplayName();
#endif
#if PLATFORM_SUPPORTS_DX11
		case ERenderAPI::DX11:    return DX11::GetDisplayName();
#endif
		default:                  return "";
		}
	}

	void RenderAPI::InitImGuiBackend()
	{
		TRACE_FUNCTION();

		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERenderAPI::OpenGL:
			{
				OpenGL::InitImGuiBackend();
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERenderAPI::DX11:
			{
				DX11::InitImGuiBackend();
				break;
			}
#endif
		}
	}

	void RenderAPI::ImGuiNewFrame()
	{
		TRACE_FUNCTION();

		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERenderAPI::OpenGL:
			{
				OpenGL::ImGuiNewFrame();
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERenderAPI::DX11:
			{
				DX11::ImGuiNewFrame();
				break;
			}
#endif
		}
	}

	void RenderAPI::ImGuiRender(ImDrawData* drawData)
	{
		TRACE_FUNCTION();

		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERenderAPI::OpenGL:
			{
				OpenGL::ImGuiRender(drawData);
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERenderAPI::DX11:
			{
				DX11::ImGuiRender(drawData);
				break;
			}
#endif
		}
	}

	void RenderAPI::ImGuiShutdown()
	{
		TRACE_FUNCTION();

		switch (s_CurrentRenderAPI)
		{
#if PLATFORM_SUPPORTS_OPENGL
			case ERenderAPI::OpenGL:
			{
				OpenGL::ImGuiShutdown();
				break;
			}
#endif
#if PLATFORM_SUPPORTS_DX11
			case ERenderAPI::DX11:
			{
				DX11::ImGuiShutdown();
				break;
			}
#endif
		}
	}

	void RenderAPI::SetCurrent(ERenderAPI api)
	{
		s_CurrentRenderAPI = api;
	}

	ERenderAPI RenderAPI::s_CurrentRenderAPI = ERenderAPI::None;
}
