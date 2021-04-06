#pragma once

#include "Renderer/VertexBuffer.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* vertices, ullong size);
		virtual ~OpenGLVertexBuffer() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetLayout(const VertexLayout& layout) const override;

		static constexpr FORCEINLINE uint VertexAttributeTypeToGLType(EVertexAttributeType type)
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

	private:
		uint m_ID;
	};
}
