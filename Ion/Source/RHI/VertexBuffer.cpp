#include "IonPCH.h"

#include "VertexBuffer.h"

namespace Ion
{
	RHIVertexLayout::RHIVertexLayout(uint32 initialAttributeCount)
		: m_Offset(0)
	{
		m_Attributes.reserve(initialAttributeCount * sizeof(VertexAttribute));
	}

	void RHIVertexLayout::AddAttribute(EVertexAttributeType attributeType, uint8 elementCount, bool bNormalized)
	{
		AddAttribute(EVertexAttributeSemantic::Null, attributeType, elementCount, bNormalized);
	}

	void RHIVertexLayout::AddAttribute(EVertexAttributeSemantic semantic, EVertexAttributeType attributeType, uint8 elementCount, bool bNormalized)
	{
		m_Attributes.push_back({ m_Offset, semantic, attributeType, elementCount, bNormalized });
		m_Offset += elementCount * GetSizeOfAttributeType(attributeType);
	}

	RHIVertexBuffer::RHIVertexBuffer()
	{
		RHILogger.Info("RHIVertexBuffer \"TODO\" object has been created.");
	}

	RHIVertexBuffer::~RHIVertexBuffer()
	{
		RHILogger.Info("RHIVertexBuffer \"TODO\" object has been destroyed.");
	}
}
