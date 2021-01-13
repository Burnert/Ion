#pragma once

#ifndef _MSC_BUILD
#error Ion can only be compiled using MSVC.
#endif
#if !(defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#error Ion can only be compiled using C++17 standard.
#endif
#ifndef UNICODE
#error Ion can only be compiled with Unicode enabled.
#endif

#include "CoreApi.h"
#include "CoreMacros.h"
#include "CoreTypes.h"
#include "CoreUtility.h"
#include "Event/Event.h"
#include "File/File.h"
#include "Input/Input.h"
#include "Layer/Layer.h"
#include "Logging/Logger.h"
#include "Profiling/DebugProfiler.h"
