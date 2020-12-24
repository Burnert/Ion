#pragma once

#include "Core/Core.h"

#ifdef ION_PLATFORM_WINDOWS
#include "Application/Platform/Windows/WindowsApplication.h"
#endif

#include "Log/Logger.h"
#include "Event/Event.h"
#include "Event/EngineEvent.h"
#include "Event/InputEvent.h"
#include "Event/WindowEvent.h"
#include "Event/EventQueue.h"

// Entry point ----------------
#include "Application/EntryPoint.h"
