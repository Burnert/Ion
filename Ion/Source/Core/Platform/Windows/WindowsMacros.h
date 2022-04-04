#pragma once

#include "Core/Platform/Windows/WindowsHeaders.h"
#include "Core/CoreMacros.h"
#include "Core/CoreAsserts.h"

#define win_check(cond, ...) \
if (!(cond)) \
{ \
	Ion::Windows::PrintLastError(__VA_ARGS__); \
	ionlocation(); \
	return; \
}

#define win_check_r(cond, ret, ...) \
if (!(cond)) \
{ \
	Ion::Windows::PrintLastError(__VA_ARGS__); \
	ionlocation(); \
	return ret; \
}

#define win_check_hresult(hr, ...) \
if (FAILED((hr))) \
{ \
	Ion::Windows::PrintHResultError(hr, __VA_ARGS__); \
	ionlocation(); \
	return; \
}

#define win_check_hresult_r(hr, ret, ...) \
if (FAILED((hr))) \
{ \
	Ion::Windows::PrintHResultError(hr, __VA_ARGS__); \
	ionlocation(); \
	return ret; \
}

#define win_check_hresult_c(hr, onfailed, ...) \
if (FAILED((hr))) \
{ \
	Ion::Windows::PrintHResultError(hr, __VA_ARGS__); \
	ionlocation(); \
	{ onfailed } \
}

#define COMRelease(ptr) if (ptr)   ((IUnknown*)ptr)->Release()
#define COMReset(ptr)   if (ptr) { ((IUnknown*)ptr)->Release(); ptr = nullptr; }
