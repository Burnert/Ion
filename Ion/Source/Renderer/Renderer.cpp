#include "IonPCH.h"

#include "Renderer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLRenderer.h"
#include "RenderAPI/DX11/DX11Renderer.h"

namespace Ion
{
	TShared<Renderer> Renderer::Create()
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLRenderer>();
		case ERenderAPI::DX11:
			return MakeShared<DX11Renderer>();
		default:
			return TShared<Renderer>(nullptr);
		}
	}
}
