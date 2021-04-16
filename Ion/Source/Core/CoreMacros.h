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

#define _CAT(a, b) a ## b
#define CAT(a, b) _CAT(a, b)

#ifdef ION_DEBUG
#define DEBUG(x) x
#else
#define DEBUG(x) ((void)0)
#endif

#define debugbreak() __debugbreak()
#define debugbreakd() DEBUG(__debugbreak())

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
