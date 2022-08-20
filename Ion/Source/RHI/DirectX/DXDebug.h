#pragma once

#include "Core/Error/Error.h"

#undef dxcall_r
#undef dxcall_t
#undef dxcall_v
#undef dxcall
#undef dxcall_f

#if ION_LOG_ENABLED

namespace Ion::_DXDebug_Detail
{
	template<typename T, typename... Args>
	FORCEINLINE String _Format(const T& format, Args&&... args)
	{
		return fmt::format(format, Forward<Args>(args)...);
	}
	FORCEINLINE String _Format()
	{
		return EmptyString;
	}

	template<typename F>
	struct FDXCall
	{
		F DXCall;

		using TRet = decltype(DXCall());

		FORCEINLINE FDXCall(F f) : DXCall(f) { }

		FORCEINLINE HRESULT operator()()
		{
			if constexpr (std::is_void_v<TRet>)
			{
				DXCall();
				return S_OK;
			}
			else
			{
				return DXCall();
			}
		}
	};
}
/**
 * Executes the call and prints messages stored by the DirectX Debug Layer.
 * Throws a DXError on FAILED(call).
 * VA_ARGS are ignored if the call's return type is void.
 */
#define dxcall_throw(call, ...) \
{ \
	Ion::g_DXDebugMessageQueue->PrepareQueue(); \
	Ion::_DXDebug_Detail::FDXCall dxCall([&] { return call; }); \
	HRESULT hResult = dxCall(); \
	if (FAILED(hResult)) \
	{ \
		Ion::g_DXDebugMessageQueue->PrintMessages(); \
		String message = Ion::_DXDebug_Detail::_Format(__VA_ARGS__); \
		String hResultMessage = Ion::Windows::FormatHResultMessage(hResult); \
		ionthrow(Ion::RHIError, "DXcall macro has thrown an error.\n{}\n{}", Move(hResultMessage), Move(message)); \
	} \
	Ion::g_DXDebugMessageQueue->PrintMessages(); \
}
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
