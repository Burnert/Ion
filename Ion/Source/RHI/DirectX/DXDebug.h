#pragma once

#include "Core/Error/Error.h"

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
 * Throws a custom Error on FAILED(call).
 * VA_ARGS are ignored if the call's return type is void.
 */
#define dxcall_throw(call, err, ...) \
{ \
	Ion::g_DXDebugMessageQueue->PrepareQueue(); \
	Ion::_DXDebug_Detail::FDXCall dxCall([&] { return call; }); \
	HRESULT hResult = dxCall(); \
	if (FAILED(hResult)) \
	{ \
		Ion::g_DXDebugMessageQueue->PrintMessages(); \
		String message = Ion::_DXDebug_Detail::_Format(__VA_ARGS__); \
		String hResultMessage = Ion::Windows::FormatHResultMessage(hResult); \
		ionthrow(err, "DXcall macro has thrown an error.\n{}\n{}", Move(hResultMessage), Move(message)); \
	} \
	Ion::g_DXDebugMessageQueue->PrintMessages(); \
}

/**
 * Executes the call and prints messages stored by the DirectX Debug Layer.
 * Throws an RHIError on FAILED(call).
 * VA_ARGS are ignored if the call's return type is void.
 */
#define dxcall(call, ...) dxcall_throw(call, Ion::RHIError, __VA_ARGS__)

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
