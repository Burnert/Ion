#pragma once

#include "WindowsCoreMacros.h"

namespace Ion
{
namespace Windows
{
	template<typename... Types>
	void PrintLastError(Types&& ...args)
	{
		DWORD lastError = GetLastError();
		WINDOWS_FORMAT_ERROR_MESSAGE(errorMsg, lastError);
		LOG_ERROR(args...);
		LOG_ERROR(std::wstring(errorMsg));
	}
}
}
