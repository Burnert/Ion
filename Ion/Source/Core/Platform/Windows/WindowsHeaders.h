#pragma once

// I don't know why Windows is like this
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
