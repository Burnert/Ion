#pragma once

#include "Core/Platform/Windows/WindowsHeaders.h"
#include "Core/CoreMacros.h"
#include "Core/Error/Error.h"

// @TODO: Refactor this to use new errors/asserts

#define win_check(cond, ...) \
if (!(cond)) \
{ \
	Ion::Windows::PrintLastError(__VA_ARGS__); \
	return; \
}

#define win_check_r(cond, ret, ...) \
if (!(cond)) \
{ \
	Ion::Windows::PrintLastError(__VA_ARGS__); \
	return ret; \
}

#define win_check_hresult(hr, ...) \
if (FAILED((hr))) \
{ \
	Ion::Windows::PrintHResultError(hr, __VA_ARGS__); \
	return; \
}

#define win_check_hresult_r(hr, ret, ...) \
if (FAILED((hr))) \
{ \
	Ion::Windows::PrintHResultError(hr, __VA_ARGS__); \
	return ret; \
}

#define win_check_hresult_c(hr, onfailed, ...) \
if (FAILED((hr))) \
{ \
	Ion::Windows::PrintHResultError(hr, __VA_ARGS__); \
	{ onfailed } \
}
