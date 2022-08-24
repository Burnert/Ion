#pragma once

// Windows doesn't #undef these if they exist.
#ifdef TEXT
#undef TEXT
#endif
#ifdef APIENTRY
#undef APIENTRY
#endif

#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <rpc.h>
#include <rpcdce.h>
