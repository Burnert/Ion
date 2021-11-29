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
		virtual void Unbind() const override;

		void BindLayout() const;

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

		// @TODO: Have to remove these as they serve no purpose.
		virtual void SetFloat(const String& name, float value) const override {}
		virtual void SetFloat2(const String& name, const Vector2& value) const override {}
		virtual void SetFloat3(const String& name, const Vector3& value) const override {}
		virtual void SetFloat4(const String& name, const Vector4& value) const override {}
		virtual void SetInt(const String& name, int32 value) const override {}
		virtual void SetInt2(const String& name, const IVector2& value) const override {}
		virtual void SetInt3(const String& name, const IVector3& value) const override {}
		virtual void SetInt4(const String& name, const IVector4& value) const override {}
		virtual void SetUInt(const String& name, uint32 value) const override {}
		virtual void SetUInt2(const String& name, const UVector2& value) const override {}
		virtual void SetUInt3(const String& name, const UVector3& value) const override {}
		virtual void SetUInt4(const String& name, const UVector4& value) const override {}
		virtual void SetMatrix2(const String& name, const Matrix2& value) const override {}
		virtual void SetMatrix2x3(const String& name, const Matrix2x3& value) const override {}
		virtual void SetMatrix2x4(const String& name, const Matrix2x4& value) const override {}
		virtual void SetMatrix3(const String& name, const Matrix3& value) const override {}
		virtual void SetMatrix3x2(const String& name, const Matrix3x2& value) const override {}
		virtual void SetMatrix3x4(const String& name, const Matrix3x4& value) const override {}
		virtual void SetMatrix4(const String& name, const Matrix4& value) const override {}
		virtual void SetMatrix4x2(const String& name, const Matrix4x2& value) const override {}
		virtual void SetMatrix4x3(const String& name, const Matrix4x3& value) const override {}

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
