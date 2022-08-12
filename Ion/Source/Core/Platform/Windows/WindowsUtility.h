#pragma once

#include "Core/Platform/Windows/WindowsHeaders.h"

#include <comdef.h>

#define WINDOWS_FORMAT_ERROR_MESSAGE(msgVarName, error) \
WCHAR _##msgVarName[512]; \
FormatMessage( \
	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
	NULL, error, 0, _##msgVarName, 512, NULL); \
WString msgVarName = _##msgVarName

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
			WindowsLogger.Error(args...);
		WindowsLogger.Error(WString(errorMsg));
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
		WindowsLogger.Error("Windows HRESULT Error:");
		if constexpr (sizeof...(args) > 0)
			WindowsLogger.Error(args...);
		WindowsLogger.Error(TString(message));
#else
		MessageBox(NULL, message, TEXT("Windows HRESULT Error"), MB_ICONERROR | MB_OK);
#endif
	}
}
}
