#pragma once

#include "WindowsHeaders.h"
#include "WindowsUtility.h"
#include "WindowsInput.h"
#include "WindowsError.h"

#define COMRelease(ptr) if (ptr)   ((IUnknown*)ptr)->Release()
#define COMReset(ptr)   if (ptr) { ((IUnknown*)ptr)->Release(); ptr = nullptr; }

namespace Ion
{
	REGISTER_LOGGER(WindowsLogger, "Platform::Windows");
}
