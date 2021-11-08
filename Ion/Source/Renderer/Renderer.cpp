#include "IonPCH.h"

#include "Renderer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLRenderer.h"

namespace Ion
{
	TShared<Renderer> Renderer::Create()
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLRenderer>();
		default:
			return TShared<Renderer>(nullptr);
		}
	}
}
