#pragma once

#include "Renderer/VertexBuffer.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* vertices, ulong size);
		virtual ~OpenGLVertexBuffer() override;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetLayout(const VertexLayout& layout) override;

		static constexpr FORCEINLINE uint VertexAttributeTypeToGLType(VertexAttributeType type)
		{
			switch (type)
			{
			case VertexAttributeType::Byte:			  return GL_BYTE;
			case VertexAttributeType::UnsignedByte:	  return GL_UNSIGNED_BYTE;
			case VertexAttributeType::Short:		  return GL_SHORT;
			case VertexAttributeType::UnsignedShort:  return GL_UNSIGNED_SHORT;
			case VertexAttributeType::Int:			  return GL_INT;
			case VertexAttributeType::UnsignedInt:	  return GL_UNSIGNED_INT;
			case VertexAttributeType::Float16:		  return GL_HALF_FLOAT;
			case VertexAttributeType::Float:		  return GL_FLOAT;
			case VertexAttributeType::Double:		  return GL_DOUBLE;
			default:                                  return 0;
			}
		}

	private:
		uint m_ID;
	};
}
