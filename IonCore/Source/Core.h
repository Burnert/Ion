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

#include "Core/CoreConfig.h"

#include "Core/Base.h"
#include "Core/Container/Tree.h"
#include "Core/Container/TreeSerializer.h"
#include "Core/Diagnostics/DebugTime.h"
#include "Core/Diagnostics/Tracing.h"
#include "Core/Error/Error.h"
#include "Core/File/File.h"
#include "Core/File/Image.h"
#include "Core/File/XML.h"
#include "Core/File/XMLParser.h"
#include "Core/File/YAML.h"
#include "Core/GUID/GUID.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/LogManager.h"
#include "Core/Math/Math.h"
#include "Core/Math/Random.h"
#include "Core/Math/Rotator.h"
#include "Core/Math/Transform.h"
#include "Core/Memory/MemoryCore.h"
#include "Core/Memory/MetaPointer.h"
#include "Core/Memory/PoolAllocator.h"
#include "Core/Memory/RefCount.h"
#include "Core/Platform/Platform.h"
#include "Core/Profiling/DebugProfiler.h"
#include "Core/Serialization/Archive.h"
#include "Core/Serialization/BinaryArchive.h"
#include "Core/Serialization/XMLArchive.h"
#include "Core/Serialization/YAMLArchive.h"
#include "Core/String/StringConverter.h"
#include "Core/String/StringUtils.h"
#include "Core/String/StringParser.h"
#include "Core/Task/EngineTaskQueue.h"
#include "Core/Task/Task.h"
#include "Core/Task/TaskFwd.h"
#include "Core/Task/TaskQueue.h"
#include "Core/Templates/StandardWrappers.h"
#include "Core/Templates/Templates.h"
