#include "IonPCH.h"

#include "IndexBuffer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLIndexBuffer.h"

namespace Ion
{
	TShared<IndexBuffer> IndexBuffer::Create(uint* indices, uint count)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLIndexBuffer>(indices, count);
		default:
			return TShared<OpenGLIndexBuffer>(nullptr);
		}
	}
}
