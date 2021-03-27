#include "IonPCH.h"

#include "VertexBuffer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLVertexBuffer.h"


namespace Ion
{
	Shared<VertexBuffer> VertexBuffer::Create(float* vertices, uint count)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLVertexBuffer>(vertices, count);
		}
		return Shared<VertexBuffer>(nullptr);
	}
}
