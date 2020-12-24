#pragma once

#include "Core/CoreTypes.h"

namespace Ion
{
	class GenericWindow
	{
	public:
		virtual ~GenericWindow();

		virtual void Initialize();

		virtual void SetTitle(WCStr title);

		virtual void SetEnabled(bool bEnabled);

		virtual void Resize();
		virtual bool GetDimensions(int& width, int& height) const;

		// Implemented per platform.
		static std::shared_ptr<GenericWindow> Create();

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		GenericWindow();
	};

	
}

