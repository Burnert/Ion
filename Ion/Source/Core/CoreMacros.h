#pragma once

#define FORCEINLINE __forceinline
#define NODISCARD [[nodiscard]]

#define BITFLAG(x) (1 << (x))

// Debug macros

#ifdef ION_DEBUG

#ifdef ION_ENGINE
#define _LOG_CRITICAL(...) ION_LOG_ENGINE_CRITICAL(__VA_ARGS__)
#else
#define _LOG_CRITICAL(...) ION_LOG_CRITICAL(__VA_ARGS__)
#endif

#define ASSERT(x) \
if (!(x)) \
{ \
	_LOG_CRITICAL("Assertion failed:\n {0}\n function: {1}\n in {2}:{3}", #x, __FUNCTION__, __FILE__, __LINE__); \
	__debugbreak(); \
}

#else

#define ASSERT(x)

#endif
