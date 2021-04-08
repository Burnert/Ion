#include "IonPCH.h"

#include "OpenGLIndexBuffer.h"

namespace Ion
{
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint* indices, uint count)
	{
		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint), indices, GL_STATIC_DRAW);
		m_Count = count;
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_ID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// IDrawable:

	void OpenGLIndexBuffer::PrepareForDraw() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}

	uint OpenGLIndexBuffer::GetIndexCount() const
	{
		return m_Count;
	}

	// End IDrawable
}
