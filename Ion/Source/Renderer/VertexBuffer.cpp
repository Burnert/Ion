#include "IonPCH.h"

#include "VertexBuffer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLVertexBuffer.h"

namespace Ion
{
	VertexLayout::VertexLayout(uint initialAttributeCount)
		: m_Offset(0)
	{
		m_Attributes.reserve(initialAttributeCount * sizeof(VertexAttribute));
	}

	void VertexLayout::AddAttribute(EVertexAttributeType attributeType, ubyte elementCount, bool normalized)
	{
		m_Attributes.push_back({ attributeType, elementCount, m_Offset, normalized });
		m_Offset += elementCount * GetSizeOfAttributeType(attributeType);
	}

	TShared<VertexBuffer> VertexBuffer::Create(void* vertices, ullong size)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLVertexBuffer>(vertices, size);
		default:
			return TShared<VertexBuffer>(nullptr);
		}
	}
}
