#pragma once

#include "Core/CoreTypes.h"

namespace Ion
{
	class GenericWindow
	{
	public:
		GenericWindow();
		virtual ~GenericWindow();

		virtual void SetTitle(WCStr title);

		virtual void SetEnabled(bool bEnabled);

		virtual void Resize();
		virtual bool GetDimensions(int& width, int& height) const;
	};
}

