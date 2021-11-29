#include "IonPCH.h"

#include "OpenGLBuffer.h"

#pragma warning(disable:6387)

namespace Ion
{
	// -------------------------------------------------------------------
	// OpenGLVertexBuffer ------------------------------------------------
	// -------------------------------------------------------------------

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertexAttributes, uint64 count)
		: m_VertexCount(0)
	{
		TRACE_FUNCTION();

		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), vertexAttributes, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		TRACE_FUNCTION();

		glDeleteBuffers(1, &m_ID);
	}

	void OpenGLVertexBuffer::SetLayout(const TShared<VertexLayout>& layout)
	{
		m_VertexLayout = layout;
	}

	uint32 OpenGLVertexBuffer::GetVertexCount() const
	{
		return m_VertexCount;
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

	void OpenGLVertexBuffer::BindLayout() const
	{
		TRACE_FUNCTION();

		uint32 attributeIndex = 0;
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

	// -------------------------------------------------------------------
	// OpenGLIndexBuffer -------------------------------------------------
	// -------------------------------------------------------------------

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

	// -------------------------------------------------------------------
	// OpenGLUniformBuffer -----------------------------------------------
	// -------------------------------------------------------------------

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
