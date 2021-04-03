#pragma once

namespace Ion
{
	enum class VertexAttributeType
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
		VertexAttributeType Type;
		ubyte ElementCount;
		ullong Offset;
		bool bNormalized = false;
	};

	class ION_API VertexLayout
	{
		friend class VertexBuffer;
	public:
		VertexLayout(uint initialAttributeCount);

		void AddAttribute(VertexAttributeType attributeType, ubyte elementCount, bool normalized = false);

		FORCEINLINE uint GetStride() const { return (uint)m_Offset; }

		FORCEINLINE const std::vector<VertexAttribute>& GetAttributes() const { return m_Attributes; }

		static constexpr FORCEINLINE ullong GetSizeOfAttributeType(const VertexAttributeType type)
		{
			switch (type)
			{
			case VertexAttributeType::Byte:			  return 1;
			case VertexAttributeType::UnsignedByte:	  return 1;
			case VertexAttributeType::Short:		  return 2;
			case VertexAttributeType::UnsignedShort:  return 2;
			case VertexAttributeType::Int:			  return 4;
			case VertexAttributeType::UnsignedInt:	  return 4;
			case VertexAttributeType::Float16:		  return 2;
			case VertexAttributeType::Float:		  return 4;
			case VertexAttributeType::Double:		  return 8;
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
