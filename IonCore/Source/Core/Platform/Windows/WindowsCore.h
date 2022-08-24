#pragma once

#include "Core/Base.h"
#include "Core/Logging/Logger.h"
#include "WindowsError.h"
#include "WindowsHeaders.h"
#include "WindowsUtility.h"

#define COMRelease(ptr) if (ptr)   ((IUnknown*)ptr)->Release()
#define COMReset(ptr)   if (ptr) { ((IUnknown*)ptr)->Release(); ptr = nullptr; }

namespace Ion
{
	REGISTER_LOGGER(WindowsLogger, "Platform::Windows");
}
