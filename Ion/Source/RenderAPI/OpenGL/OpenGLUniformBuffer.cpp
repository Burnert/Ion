#include "IonPCH.h"

#include "OpenGLUniformBuffer.h"

namespace Ion
{
#pragma warning(disable:6387)
	OpenGLUniformBuffer::OpenGLUniformBuffer(void* initialData, size_t size) :
		m_Data(nullptr),
		m_DataSize(size),
		m_ID(0)
	{
		TRACE_FUNCTION();

		ionassert(initialData);
		ionassert(size > 0);

		m_Data = _aligned_malloc(size, 16);
		memcpy(m_Data, initialData, size);

		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
		glBufferData(GL_UNIFORM_BUFFER, size, m_Data, GL_STATIC_DRAW);
	}

	void* OpenGLUniformBuffer::GetDataPtr() const
	{
		return m_Data;
	}

	void OpenGLUniformBuffer::UpdateData() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, m_DataSize, m_Data);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		TRACE_FUNCTION();

		_aligned_free(m_Data);

		glDeleteBuffers(1, &m_ID);
	}

	void OpenGLUniformBuffer::Bind(uint32 slot) const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_ID);
	}
}
