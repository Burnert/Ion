#include "IonPCH.h"

#include "OpenGLVertexBuffer.h"

namespace Ion
{
	OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, ulong size)
	{
		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_ID);
	}

	void OpenGLVertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}

	void OpenGLVertexBuffer::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetLayout(const VertexLayout& layout)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);

		uint attributeIndex = 0;
		for (const VertexAttribute& attribute : layout.GetAttributes())
		{
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
}
