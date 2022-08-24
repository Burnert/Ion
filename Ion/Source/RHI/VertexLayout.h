#pragma once

namespace Ion
{
	class ION_API RHIVertexLayout : public RefCountable
	{
	public:
		RHIVertexLayout(uint32 initialAttributeCount);

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

		friend class RHIVertexBuffer;
	};
}
