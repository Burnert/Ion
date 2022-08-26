#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_OPENGL

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

	void OpenGLVertexBuffer::SetLayout(const TRef<RHIVertexLayout>& layout)
	{
		m_VertexLayout = layout;
	}

	Result<void, RHIError> OpenGLVertexBuffer::SetLayoutShader(const TRef<RHIShader>& shader)
	{
		return Ok();
	}

	uint32 OpenGLVertexBuffer::GetVertexCount() const
	{
		return m_VertexCount;
	}

	Result<void, RHIError> OpenGLVertexBuffer::Bind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_ID);

		return Ok();
	}

	Result<void, RHIError> OpenGLVertexBuffer::Unbind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return Ok();
	}

	Result<void, RHIError> OpenGLVertexBuffer::BindLayout() const
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

		return Ok();
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

	Result<void, RHIError> OpenGLIndexBuffer::Bind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);

		return Ok();
	}

	Result<void, RHIError> OpenGLIndexBuffer::Unbind() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return Ok();
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

	Result<void, RHIError> OpenGLUniformBuffer::UpdateData() const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, m_DataSize, m_Data);

		return Ok();
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		TRACE_FUNCTION();

		_aligned_free(m_Data);

		glDeleteBuffers(1, &m_ID);
	}

	Result<void, RHIError> OpenGLUniformBuffer::Bind(uint32 slot) const
	{
		TRACE_FUNCTION();

		glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_ID);

		return Ok();
	}
}

#endif // RHI_BUILD_OPENGL
