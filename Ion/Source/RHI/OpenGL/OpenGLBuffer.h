#pragma once

#include "OpenGL.h"
#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "RHI/UniformBuffer.h"

namespace Ion
{
	class ION_API OpenGLVertexBuffer : public RHIVertexBuffer
	{
		friend class OpenGLRenderer;
	public:
		OpenGLVertexBuffer(float* vertexAttributes, uint64 count);
		virtual ~OpenGLVertexBuffer() override;

		virtual void SetLayout(const std::shared_ptr<RHIVertexLayout>& layout) override;
		virtual Result<void, RHIError> SetLayoutShader(const std::shared_ptr<RHIShader>& shader) override;

		virtual uint32 GetVertexCount() const override;

		static constexpr FORCEINLINE uint32 VertexAttributeTypeToGLType(EVertexAttributeType type)
		{
			switch (type)
			{
			case EVertexAttributeType::Byte:           return GL_BYTE;
			case EVertexAttributeType::UnsignedByte:   return GL_UNSIGNED_BYTE;
			case EVertexAttributeType::Short:          return GL_SHORT;
			case EVertexAttributeType::UnsignedShort:  return GL_UNSIGNED_SHORT;
			case EVertexAttributeType::Int:            return GL_INT;
			case EVertexAttributeType::UnsignedInt:    return GL_UNSIGNED_INT;
			case EVertexAttributeType::Float16:        return GL_HALF_FLOAT;
			case EVertexAttributeType::Float:          return GL_FLOAT;
			case EVertexAttributeType::Double:         return GL_DOUBLE;
			default:                                   return 0;
			}
		}

	protected:
		virtual Result<void, RHIError> Bind() const override;
		virtual Result<void, RHIError> BindLayout() const override;
		virtual Result<void, RHIError> Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_VertexCount;
		std::shared_ptr<RHIVertexLayout> m_VertexLayout;
	};

	class ION_API OpenGLIndexBuffer : public RHIIndexBuffer
	{
		friend class OpenGLRenderer;
	public:
		OpenGLIndexBuffer(uint32* indices, uint32 count);
		virtual ~OpenGLIndexBuffer() override;

		virtual uint32 GetIndexCount() const override;
		virtual uint32 GetTriangleCount() const override;

	protected:
		virtual Result<void, RHIError> Bind() const override;
		virtual Result<void, RHIError> Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_Count;
		uint32 m_TriangleCount;
	};

	class ION_API OpenGLUniformBuffer : public RHIUniformBuffer
	{
	public:
		virtual ~OpenGLUniformBuffer() override;

		virtual Result<void, RHIError> Bind(uint32 slot = 0) const override;

	protected:
		OpenGLUniformBuffer(void* initialData, size_t size);
		//OpenGLUniformBuffer(void* data, size_t size, const UniformDataMap& uniforms);

		virtual void* GetDataPtr() const override;
		virtual Result<void, RHIError> UpdateData() const override;

	private:
		void* m_Data;
		size_t m_DataSize;
		uint32 m_ID;

		friend class OpenGLRenderer;
		friend class RHIUniformBuffer;
	};
}
