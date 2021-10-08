#pragma once

#include "Core/Core.h"
#include "Core/Event/Event.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Event/InputEvent.h"
#include "Core/Event/WindowEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Layer/Layer.h"
#include "Core/Layer/LayerStack.h"
#include "Core/Logging/Logger.h"

#ifdef ION_PLATFORM_WINDOWS
#include "Application/Platform/Windows/WindowsApplication.h"
#endif

// Entry point ----------------
#include "Application/EntryPoint.h"
