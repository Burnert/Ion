#include "IonPCH.h"

#include "OpenGLIndexBuffer.h"

namespace Ion
{
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint* indices, uint count)
	{
		TRACE_FUNCTION();

		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint), indices, GL_STATIC_DRAW);
		m_Count = count;
		m_TriangleCount = count / 3;
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		TRACE_FUNCTION();

		glDeleteBuffers(1, &m_ID);
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

	uint OpenGLIndexBuffer::GetIndexCount() const
	{
		return m_Count;
	}

	uint OpenGLIndexBuffer::GetTriangleCount() const
	{
		return m_TriangleCount;
	}
}
