#pragma once

#include "WindowsMacros.h"
#include "Core/Logging/Logger.h"

#include <comdef.h>

namespace Ion
{
namespace Windows
{
	template<typename... Types>
	void PrintError(DWORD error, Types&& ...args)
	{
		WINDOWS_FORMAT_ERROR_MESSAGE(errorMsg, error);
#ifdef ION_LOG_ENABLED
		if constexpr (sizeof...(args) > 0)
			LOG_ERROR(args...);
		LOG_ERROR(WString(errorMsg));
#else
		// @TODO: Make this show up only on critical errors
		MessageBox(NULL, errorMsg.c_str(), TEXT("Win32 Critical Error"), MB_ICONERROR | MB_OK);
#endif
	}

	template<typename... Types>
	void PrintLastError(Types&& ...args)
	{
		PrintError(GetLastError(), args...);
	}

	template<typename... Types>
	void PrintHResultError(HRESULT hr, Types&& ...args)
	{
		_com_error error(hr);
		const TCHAR* message = error.ErrorMessage();

#ifdef ION_LOG_ENABLED
		if constexpr (sizeof...(args) > 0)
			LOG_ERROR(args...);
		LOG_ERROR(TString(message));
#else
		MessageBox(NULL, message, TEXT("Win32 HRESULT Error"), MB_ICONERROR | MB_OK);
#endif
	}
}
}
