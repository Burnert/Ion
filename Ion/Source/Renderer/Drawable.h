#pragma once

#include "Core/CoreTypes.h"

namespace Ion
{
	// @TODO: Think of something more robust.

	class IDrawable
	{
	public:
		virtual void PrepareForDraw() const = 0;
		virtual uint GetIndexCount() const = 0;
	};
}
