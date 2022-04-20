#pragma once

#include "Core/CoreTypes.h"

namespace Ion
{
	enum class EVertexAttributeType : uint8
	{
		Null = 0,
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

	enum class EVertexAttributeSemantic : uint8
	{
		Null = 0,
		Position,
		Normal,
		TexCoord,
	};

	struct VertexAttribute
	{
		uint64 Offset;
		EVertexAttributeSemantic Semantic;
		EVertexAttributeType Type;
		uint8 ElementCount;
		bool bNormalized = false;
	};
}
