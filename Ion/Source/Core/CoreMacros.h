#pragma once

#include "Core/Logging/Logger.h"

#define FORCEINLINE __forceinline
#define NODISCARD [[nodiscard]]

#define BITFLAG(x) (1 << (x))

#define BIND_METHOD(x)    std::bind(&x, this)
#define BIND_METHOD_1P(x) std::bind(&x, this, std::placeholders::_1)

#undef TEXT
#ifdef UNICODE
#define TEXT(x) L##x
#else
#define TEXT(x) x
#endif

// @TODO: Handle the verify failure with some debug code instead of break in the future

#define VERIFY(x) \
if (!(x)) \
{ \
	LOG_CRITICAL("Verification failed!:\n {0}\n function: {1}\n in {2}:{3}", #x, __FUNCTION__, __FILE__, __LINE__); \
	__debugbreak(); \
}

#define VERIFY_M(x, message) \
if (!(x)) \
{ \
	LOG_CRITICAL(message##"\n {0}\n function: {1}\n in {2}:{3}", #x, __FUNCTION__, __FILE__, __LINE__); \
	__debugbreak(); \
}

// Debug macros

#ifdef ION_DEBUG

// @TODO: Implement a nice error reporting system instead of this spaghetti

#define ASSERT(x) \
if (!(x)) \
{ \
	LOG_CRITICAL("Assertion failed:\n {0}\n function: {1}\n in {2}:{3}", #x, __FUNCTION__, __FILE__, __LINE__); \
	__debugbreak(); \
}

#define ASSERT_M(x, message) \
if (!(x)) \
{ \
	LOG_CRITICAL(message##"\n {0}\n function: {1}\n in {2}:{3}", #x, __FUNCTION__, __FILE__, __LINE__); \
	__debugbreak(); \
}

#define BREAK() __debugbreak();

#define BREAK_M(x) \
{ \
	LOG_CRITICAL(x); \
	__debugbreak(); \
}

#define DEBUG(x) x

#else

#define ASSERT(x)
#define ASSERT_M(x, message)
#define BREAK()
#define BREAK_M(x)
#define DEBUG(x)

#endif

#ifdef ION_PLATFORM_WINDOWS
	#define PLATFORM_SUPPORTS_OPENGL 1
	#define PLATFORM_SUPPORTS_DIRECTX 1
	#define PLATFORM_SUPPORTS_DX12 1
	#define PLATFORM_SUPPORTS_VULKAN 1

	#define PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL 0
#elif ION_PLATFORM_LINUX
	#define PLATFORM_SUPPORTS_OPENGL 1
	#define PLATFORM_SUPPORTS_DIRECTX 0
	#define PLATFORM_SUPPORTS_DX12 0
	#define PLATFORM_SUPPORTS_VULKAN 1

	#define PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL 1
#endif
