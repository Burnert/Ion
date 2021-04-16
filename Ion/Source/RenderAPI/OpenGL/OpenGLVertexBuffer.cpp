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

	void OpenGLVertexBuffer::SetLayout(const VertexLayout& layout) const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_ID);

		uint attributeIndex = 0;
		for (const VertexAttribute& attribute : layout.GetAttributes())
		{
			TRACE_SCOPE("OpenGLVertexBuffer - Setup layout");

			glVertexAttribPointer(attributeIndex, 
				attribute.ElementCount, 
				VertexAttributeTypeToGLType(attribute.Type), 
				attribute.bNormalized,
				layout.GetStride(), 
				(const void*)attribute.Offset);
			glEnableVertexAttribArray(attributeIndex);
			attributeIndex++;
		}
	}

	uint OpenGLVertexBuffer::GetVertexCount() const
	{
		// @TODO: Vertex count is not set anywhere
		return m_VertexCount;
	}
}
