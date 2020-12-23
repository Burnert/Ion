#pragma once

#ifndef _MSC_BUILD
#error Ion can only be compiled using MSVC.
#elif !(defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#error Ion can only be compiled using C++17 standard.
#endif

#include "CoreApi.h"
#include "CoreMacros.h"
#include "CoreTypes.h"

#include "Log/Logger.h"

#include "Event/Event.h"