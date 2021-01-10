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
#ifdef ION_LOG_ENABLED
		LOG_ERROR(args...);
		LOG_ERROR(std::wstring(errorMsg));
#else
		// @TODO: Make this show up only on critical errors
		MessageBox(NULL, errorMsg.c_str(), TEXT("Win32 Critical Error"), MB_ICONERROR | MB_OK);
#endif
	}
}
}
