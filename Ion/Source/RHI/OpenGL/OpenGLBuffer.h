#pragma once

#include "OpenGL.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/UniformBuffer.h"

namespace Ion
{
	class ION_API OpenGLVertexBuffer : public VertexBuffer
	{
		friend class OpenGLRenderer;
	public:
		OpenGLVertexBuffer(float* vertexAttributes, uint64 count);
		virtual ~OpenGLVertexBuffer() override;

		virtual void SetLayout(const TShared<VertexLayout>& layout) override;
		virtual void SetLayoutShader(const TShared<Shader>& shader) override;

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
		virtual void Bind() const override;
		virtual void BindLayout() const override;
		virtual void Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_VertexCount;
		TShared<VertexLayout> m_VertexLayout;
	};

	class ION_API OpenGLIndexBuffer : public IndexBuffer
	{
		friend class OpenGLRenderer;
	public:
		OpenGLIndexBuffer(uint32* indices, uint32 count);
		virtual ~OpenGLIndexBuffer() override;

		virtual uint32 GetIndexCount() const override;
		virtual uint32 GetTriangleCount() const override;

	protected:
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint32 m_ID;
		uint32 m_Count;
		uint32 m_TriangleCount;
	};

	class ION_API OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		virtual ~OpenGLUniformBuffer() override;

		virtual void Bind(uint32 slot = 0) const override;

	protected:
		OpenGLUniformBuffer(void* initialData, size_t size);
		//OpenGLUniformBuffer(void* data, size_t size, const UniformDataMap& uniforms);

		virtual void* GetDataPtr() const override;
		virtual void UpdateData() const override;

	private:
		void* m_Data;
		size_t m_DataSize;
		uint32 m_ID;

		friend class OpenGLRenderer;
		friend class UniformBuffer;
	};
}
