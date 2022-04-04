#pragma once

#include "IonApp.h"

// Entry point ----------------
#include "Application/EntryPoint.h"

#if ION_PLATFORM_WINDOWS
#include "Application/Platform/Windows/WindowsApplication.h"
using PlatformApp = Ion::WindowsApplication;
#else
#error Other platforms unsupported.
#endif

/* Specifies the main class of the application (can be used only once) */
#define USE_APPLICATION_CLASS(className) \
namespace Ion \
{ \
	Application* InstantiateApplication() \
	{ \
		className* clientApp = new className; \
		Application* engineApp = Application::Create<PlatformApp>(clientApp); \
		clientApp->m_EngineApplication = engineApp; \
		return engineApp; \
	} \
}
