#include "IonPCH.h"

#include "IndexBuffer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLIndexBuffer.h"
#include "RenderAPI/DX11/DX11Buffer.h"

namespace Ion
{
	TShared<IndexBuffer> IndexBuffer::Create(uint32* indices, uint32 count)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLIndexBuffer>(indices, count);
		case ERenderAPI::DX11:
			return MakeShared<DX11IndexBuffer>(indices, count);
		default:
			return TShared<IndexBuffer>(nullptr);
		}
	}
}
