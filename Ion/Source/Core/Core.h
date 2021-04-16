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

#include "Config.h"

#include "Core/CoreApi.h"
#include "Core/CoreAsserts.h"
#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"
#include "Core/CoreUtility.h"
#include "Core/Math/Math.h"
#include "Core/Event/Event.h"
#include "Core/StringUtils.h"
#include "Core/StringConverter.h"
#include "Core/File/File.h"
#include "Core/File/Image.h"
#include "Core/Input/Input.h"
#include "Core/Layer/Layer.h"
#include "Core/Logging/Logger.h"
#include "Core/Profiling/DebugProfiler.h"
#include "Core/Serialisation/Serialisation.h"
#include "Core/Templates/Templates.h"
#include "Core/Diagnostics/Tracing.h"
