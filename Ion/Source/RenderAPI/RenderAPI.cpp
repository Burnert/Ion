#include "IonPCH.h"

#include "RenderAPI.h"
#include "OpenGL/OpenGL.h"

DECLARE_PERFORMANCE_COUNTER(RenderAPI_InitTime, "RenderAPI Init Time", "Init");

namespace Ion
{
	bool RenderAPI::Init(ERenderAPI api)
	{
		SCOPED_PERFORMANCE_COUNTER(RenderAPI_InitTime);

		switch (api)
		{
		case Ion::ERenderAPI::OpenGL:
			SetCurrent(ERenderAPI::OpenGL);
			OpenGL::InitOpenGL();
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
		}
	}

	void RenderAPI::SetCurrent(ERenderAPI api)
	{
		m_CurrentRenderAPI = api;
	}

	ERenderAPI RenderAPI::m_CurrentRenderAPI = ERenderAPI::None;
}
