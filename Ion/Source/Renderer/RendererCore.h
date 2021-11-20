#pragma once

#include "Core/CoreTypes.h"

#define UNIFORMBUFFER alignas(16)

namespace Ion
{
	struct ViewportDimensions
	{
		int32 X;
		int32 Y;
		int32 Width;
		int32 Height;
	};

	enum class EPolygonDrawMode : uint8
	{
		Fill,
		Lines,
		Points,
	};
}
