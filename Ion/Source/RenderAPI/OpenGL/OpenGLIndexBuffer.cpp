#include "IonPCH.h"

#include "OpenGLIndexBuffer.h"

namespace Ion
{
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32* indices, uint32 count)
	{
		TRACE_FUNCTION();

		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32), indices, GL_STATIC_DRAW);
		m_Count = count;
		m_TriangleCount = count / 3;
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		TRACE_FUNCTION();

		glDeleteBuffers(1, &m_ID);
	}

	uint32 OpenGLIndexBuffer::GetIndexCount() const
	{
		return m_Count;
	}

	uint32 OpenGLIndexBuffer::GetTriangleCount() const
	{
		return m_TriangleCount;
	}

	void OpenGLIndexBuffer::Bind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
