#pragma once

namespace Ion
{
	enum class EVertexAttributeType
	{
		Null,
		Byte,
		UnsignedByte,
		Short,
		UnsignedShort,
		Int,
		UnsignedInt,
		Float16,
		Float,
		Double,
	};

	struct VertexAttribute
	{
		EVertexAttributeType Type;
		ubyte ElementCount;
		ullong Offset;
		bool bNormalized = false;
	};

	class ION_API VertexLayout
	{
		friend class VertexBuffer;
	public:
		VertexLayout(uint initialAttributeCount);

		void AddAttribute(EVertexAttributeType attributeType, ubyte elementCount, bool normalized = false);

		FORCEINLINE uint GetStride() const { return (uint)m_Offset; }

		FORCEINLINE const std::vector<VertexAttribute>& GetAttributes() const { return m_Attributes; }

		static constexpr FORCEINLINE ullong GetSizeOfAttributeType(const EVertexAttributeType type)
		{
			switch (type)
			{
			case EVertexAttributeType::Byte:			  return 1;
			case EVertexAttributeType::UnsignedByte:	  return 1;
			case EVertexAttributeType::Short:		  return 2;
			case EVertexAttributeType::UnsignedShort:  return 2;
			case EVertexAttributeType::Int:			  return 4;
			case EVertexAttributeType::UnsignedInt:	  return 4;
			case EVertexAttributeType::Float16:		  return 2;
			case EVertexAttributeType::Float:		  return 4;
			case EVertexAttributeType::Double:		  return 8;
			default:                                  return 0;
			}
		}

	private:
		ullong m_Offset;
		std::vector<VertexAttribute> m_Attributes;
	};

	class ION_API VertexBuffer
	{
	public:
		static Shared<VertexBuffer> Create(void* vertices, uint size);

		virtual ~VertexBuffer() { };

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetLayout(const VertexLayout& layout) = 0;

	protected:
		VertexBuffer() { }
	};
}
