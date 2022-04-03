#pragma once

#include "Core/CoreTypes.h"

#define UNIFORMBUFFER alignas(16)

#if ION_FORCE_SHADER_DEBUG || (ION_DEBUG && ION_ENABLE_SHADER_DEBUG)
#define SHADER_DEBUG_ENABLED 1
#else
#define SHADER_DEBUG_ENABLED 0
#endif

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
