#include "IonPCH.h"

#include "VertexBuffer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLVertexBuffer.h"
#include "RenderAPI/DX11/DX11Buffer.h"

namespace Ion
{
	VertexLayout::VertexLayout(uint32 initialAttributeCount)
		: m_Offset(0)
	{
		m_Attributes.reserve(initialAttributeCount * sizeof(VertexAttribute));
	}

	void VertexLayout::AddAttribute(EVertexAttributeType attributeType, uint8 elementCount, bool bNormalized)
	{
		AddAttribute(EVertexAttributeSemantic::Null, attributeType, elementCount, bNormalized);
	}

	void VertexLayout::AddAttribute(EVertexAttributeSemantic semantic, EVertexAttributeType attributeType, uint8 elementCount, bool bNormalized)
	{
		m_Attributes.push_back({ m_Offset, semantic, attributeType, elementCount, bNormalized });
		m_Offset += elementCount * GetSizeOfAttributeType(attributeType);
	}

	TShared<VertexBuffer> VertexBuffer::Create(float* vertexAttributes, uint64 count)
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLVertexBuffer>(vertexAttributes, count);
		case ERenderAPI::DX11:
			return MakeShared<DX11VertexBuffer>(vertexAttributes, count);
		default:
			return TShared<VertexBuffer>(nullptr);
		}
	}
}
