#pragma once

#include "Core/Logging/Logger.h"

#define FORCEINLINE __forceinline
#define NODISCARD [[nodiscard]]
#define NOVTABLE __declspec(novtable)

#undef TEXT
#ifdef UNICODE
#define TEXT(x) L##x
#else
#define TEXT(x) x
#endif

#define _CAT(a, b) a ## b
#define CAT(a, b) _CAT(a, b)

#define BITFLAG(x) (1 << (x))

/* If the condition is 1, return the bitflags, else return 0.
   [If] must be the same type as [Bits] */
#define FLAGS_IF(If, Bits) (~((If) - 1) & (Bits))

#define BIND_METHOD(x)    std::bind(&x, this)
#define BIND_METHOD_1P(x) std::bind(&x, this, std::placeholders::_1)

/* Calls the code outside of a function
   Place expr in { }
   id param should be something unique in the translation unit.
   Also, it's probably a bad idea to put this in header files,
   unless you want it to be called every time they're included in a cpp. */
#define CALL_OUTSIDE(expr, id) \
static int _CallOutside##id = ([]expr(), 0)

#ifdef ION_DEBUG
#define DEBUG(x) x
#else
#define DEBUG(x) ((void)0)
#endif

#define debugbreak() __debugbreak()
#define debugbreakd() DEBUG(__debugbreak())

#define checked_call(func, ...) if (func) func(__VA_ARGS__)

#define checked_delete(ptr) if (ptr) delete ptr

#if ION_PLATFORM_WINDOWS
	#define PLATFORM_SUPPORTS_OPENGL 1
	#define PLATFORM_SUPPORTS_DX11 1
	#define PLATFORM_SUPPORTS_DX12 1
	#define PLATFORM_SUPPORTS_VULKAN 1

	#define PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL 1
#elif ION_PLATFORM_LINUX
	#define PLATFORM_SUPPORTS_OPENGL 1
	#define PLATFORM_SUPPORTS_DX11 0
	#define PLATFORM_SUPPORTS_DX12 0
	#define PLATFORM_SUPPORTS_VULKAN 1

	#define PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL 1
#else
	#define PLATFORM_SUPPORTS_OPENGL 0
	#define PLATFORM_SUPPORTS_DX11 0
	#define PLATFORM_SUPPORTS_DX12 0
	#define PLATFORM_SUPPORTS_VULKAN 0

	define PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL 0
#endif
