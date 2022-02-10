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
#include "Core/Math/Transform.h"
#include "Core/Math/Rotator.h"
#include "Core/Math/Random.h"
#include "Core/Event/Event.h"
#include "Core/StringUtils.h"
#include "Core/StringConverter.h"
#include "Core/Asset/AssetCore.h"
#include "Core/File/File.h"
#include "Core/File/Image.h"
#include "Core/GUID.h"
#include "Core/Input/Input.h"
#include "Core/Layer/Layer.h"
#include "Core/Logging/Logger.h"
#include "Core/Memory/MemoryCore.h"
//#include "Core/Memory/PoolAllocator.h"
#include "Core/Profiling/DebugProfiler.h"
#include "Core/Serialisation/Serialisation.h"
#include "Core/Templates/Templates.h"
#include "Core/Diagnostics/Tracing.h"
#include "Core/Diagnostics/DebugTime.h"
#include "Renderer/RendererFwd.h"
#include "Engine/EngineFwd.h"
