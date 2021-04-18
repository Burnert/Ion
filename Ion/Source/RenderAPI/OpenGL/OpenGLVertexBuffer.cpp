#include "IonPCH.h"

#include "OpenGLVertexBuffer.h"

namespace Ion
{
	OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, ullong size)
	{
		TRACE_FUNCTION();

		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		TRACE_FUNCTION();

		glDeleteBuffers(1, &m_ID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetLayout(const VertexLayout& layout)
	{
		m_VertexLayout = MakeShareable(new VertexLayout(layout));
	}

	uint OpenGLVertexBuffer::GetVertexCount() const
	{
		// @TODO: Vertex count is not set anywhere
		return m_VertexCount;
	}

	void OpenGLVertexBuffer::BindLayout() const
	{
		TRACE_FUNCTION();

		uint attributeIndex = 0;
		for (const VertexAttribute& attribute : m_VertexLayout->GetAttributes())
		{
			glVertexAttribPointer(attributeIndex,
				attribute.ElementCount,
				VertexAttributeTypeToGLType(attribute.Type),
				attribute.bNormalized,
				m_VertexLayout->GetStride(),
				(const void*)attribute.Offset);
			glEnableVertexAttribArray(attributeIndex);
			attributeIndex++;
		}
	}
}
