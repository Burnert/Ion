#pragma once

#include "Log/Logger.h"

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

// Debug macros

#ifdef ION_DEBUG

#define ASSERT(x) \
if (!(x)) \
{ \
	LOG_CRITICAL("Assertion failed:\n {0}\n function: {1}\n in {2}:{3}", #x, __FUNCTION__, __FILE__, __LINE__); \
	__debugbreak(); \
}

#define DEBUG(x) x

#else

#define ASSERT(x)
#define DEBUG(x)

#endif
