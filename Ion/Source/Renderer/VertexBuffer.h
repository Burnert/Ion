#pragma once

#include "VertexAttribute.h"

namespace Ion
{
	class Shader;

	class ION_API VertexLayout
	{
		friend class VertexBuffer;
	public:
		VertexLayout(uint32 initialAttributeCount);

		void AddAttribute(EVertexAttributeType attributeType, uint8 elementCount, bool bNormalized = false);
		void AddAttribute(EVertexAttributeSemantic semantic, EVertexAttributeType attributeType, uint8 elementCount, bool bNormalized = false);

		FORCEINLINE uint32 GetStride() const { return (uint32)m_Offset; }

		FORCEINLINE const TArray<VertexAttribute>& GetAttributes() const { return m_Attributes; }

		static constexpr FORCEINLINE uint64 GetSizeOfAttributeType(const EVertexAttributeType type)
		{
			switch (type)
			{
			case EVertexAttributeType::Byte:           return 1;
			case EVertexAttributeType::UnsignedByte:   return 1;
			case EVertexAttributeType::Short:          return 2;
			case EVertexAttributeType::UnsignedShort:  return 2;
			case EVertexAttributeType::Int:            return 4;
			case EVertexAttributeType::UnsignedInt:    return 4;
			case EVertexAttributeType::Float16:        return 2;
			case EVertexAttributeType::Float:          return 4;
			case EVertexAttributeType::Double:         return 8;
			default:                                   return 0;
			}
		}

	private:
		uint64 m_Offset;
		TArray<VertexAttribute> m_Attributes;
	};

	class ION_API VertexBuffer
	{
	public:
		static TShared<VertexBuffer> Create(float* vertexAttributes, uint64 count);

		virtual ~VertexBuffer() { }

		virtual void SetLayout(const TShared<VertexLayout>& layout, const TShared<Shader>& shader) = 0;

		virtual uint32 GetVertexCount() const = 0;

	protected:
		VertexBuffer() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}
