#pragma once

#undef dxcall_r
#undef dxcall_t
#undef dxcall_v
#undef dxcall
#undef dxcall_f

#if ION_LOG_ENABLED
/**
 * Requires 'HRESULT hResult;' in function scope.
 *
 * Returns a custom value on error.
 */
#define dxcall_r(call, ret, ...) \
{ \
	Ion::g_DXDebugMessageQueue->PrepareQueue(); \
	hResult = call; \
	win_check_hresult_c(hResult, { Ion::g_DXDebugMessageQueue->PrintMessages(); debugbreak(); return ret; }, __VA_ARGS__) \
	Ion::g_DXDebugMessageQueue->PrintMessages(); \
}
/**
 * Requires 'HRESULT hResult;' in function scope.
 *
 * Throws an Error on error.
 */
#define dxcall_t(call, err, ...) \
{ \
	Ion::g_DXDebugMessageQueue->PrepareQueue(); \
	hResult = call; \
	win_check_hresult_c(hResult, { Ion::g_DXDebugMessageQueue->PrintMessages(); debugbreak(); err; }, __VA_ARGS__) \
	Ion::g_DXDebugMessageQueue->PrintMessages(); \
}
#define dxcall_v(call, ...) \
{ \
	Ion::g_DXDebugMessageQueue->PrepareQueue(); \
	call; \
	Ion::g_DXDebugMessageQueue->PrintMessages(); \
}
#else
#define dxcall_r(call, ret, ...) hResult = call
#define dxcall_t(call, err, ...) hResult = call
#define dxcall_v(call, ...) call
#endif

/**
 * Requires 'HRESULT hResult;' in function scope.
 */
#define dxcall(call, ...) dxcall_r(call, , __VA_ARGS__)
/**
 * Requires 'HRESULT hResult;' in function scope.
 *
 * Returns false on error.
 */
#define dxcall_f(call, ...) dxcall_r(call, false, __VA_ARGS__)

namespace Ion
{
	class IDXDebugMessageQueue
	{
	public:
		virtual void PrepareQueue() = 0;
		virtual void PrintMessages() = 0;
	};

	inline IDXDebugMessageQueue* g_DXDebugMessageQueue = nullptr;
}
