#pragma once

#include "Core/Core.h"

#ifdef ION_PLATFORM_WINDOWS
#include "Application/Platform/Windows/WindowsApplication.h"
#endif

#include "Log/Logger.h"
#include "Core/Event/Event.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Event/EngineEvent.h"
#include "Core/Event/InputEvent.h"
#include "Core/Event/WindowEvent.h"
#include "Core/Event/EventQueue.h"

// Entry point ----------------
#include "Application/EntryPoint.h"
