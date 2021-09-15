#include "IonPCH.h"

#include "RenderAPI.h"
#include "OpenGL/OpenGL.h"

//DECLARE_PERFORMANCE_COUNTER(RenderAPI_InitTime, "RenderAPI Init Time", "Init");

namespace Ion
{
	bool RenderAPI::Init(ERenderAPI api)
	{
		//SCOPED_PERFORMANCE_COUNTER(RenderAPI_InitTime);
		TRACE_FUNCTION();

		switch (api)
		{
		case Ion::ERenderAPI::OpenGL:
			SetCurrent(ERenderAPI::OpenGL);
			OpenGL::Init();
			break;
		default:
			return false;
		}
		return true;
	}

	const char* RenderAPI::GetCurrentDisplayName()
	{
		switch (m_CurrentRenderAPI)
		{
		case ERenderAPI::None:
			return "Currently not using any Render API.";
		case ERenderAPI::OpenGL: 
			return OpenGL::GetDisplayName();
		default:
			return "";
		}
	}

	void RenderAPI::InitImGuiBackend()
	{
		TRACE_FUNCTION();

		switch (m_CurrentRenderAPI)
		{
		case ERenderAPI::OpenGL: 
			OpenGL::InitImGuiBackend();
		}
	}

	void RenderAPI::ImGuiNewFrame()
	{
		TRACE_FUNCTION();

		switch (m_CurrentRenderAPI)
		{
		case ERenderAPI::OpenGL:
			OpenGL::ImGuiNewFrame();
		}
	}

	void RenderAPI::ImGuiRender(ImDrawData* drawData)
	{
		TRACE_FUNCTION();

		switch (m_CurrentRenderAPI)
		{
		case ERenderAPI::OpenGL:
			OpenGL::ImGuiRender(drawData);
		}
	}

	void RenderAPI::ImGuiShutdown()
	{
		TRACE_FUNCTION();

		switch (m_CurrentRenderAPI)
		{
		case ERenderAPI::OpenGL:
			OpenGL::ImGuiShutdown();
		}
	}

	void RenderAPI::SetCurrent(ERenderAPI api)
	{
		m_CurrentRenderAPI = api;
	}

	ERenderAPI RenderAPI::m_CurrentRenderAPI = ERenderAPI::None;
}
