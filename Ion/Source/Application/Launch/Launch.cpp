#include "IonPCH.h"

#if ION_PLATFORM_WINDOWS
#include "Application/Platform/Windows/WindowsApplication.h"
#else
#error Current platform is not supported.
#endif

#include "Application/Application.h"
#include "IonApp.h"

namespace Ion
{
	void ParseCommandLineArgs(int32 argc, tchar* argv[])
	{
		// @TODO: Save engine path in system environment variables or something

		for (int32 i = 0; i < argc; ++i)
		{
			bool bHasNextArg = i + 1 < argc;
			tchar* arg = argv[i];
			tchar* nextArg = bHasNextArg ? argv[i + 1] : nullptr;
			if ((
				tstrcmp(arg, TEXT("--enginePath")) == 0 ||
				tstrcmp(arg, TEXT("-e")) == 0
				) && bHasNextArg)
			{
				EnginePath::SetEnginePath(nextArg);
				++i;
			}
		}
	}

	int32 MainShared(int32 argc, tchar* argv[])
	{
		ParseCommandLineArgs(argc, argv);

#if ION_ENABLE_TRACING
		DebugTracing::Init();
#endif
		DebugTimer::InitPlatform();

		Platform::Internal::SetMainThreadId();

		Platform::SetConsoleOutputUTF8();

		// Init
		TRACE_SESSION_BEGIN("Init");
		TRACE_RECORD_START();

		g_pEngineApplication->Init();

		TRACE_RECORD_STOP();
		TRACE_SESSION_END();

		// Run
		TRACE_SESSION_BEGIN("Run");

		g_pEngineApplication->RunLoop();

		TRACE_SESSION_END();

		// Shutdown
		TRACE_SESSION_BEGIN("Shutdown");
		TRACE_RECORD_START();

		g_pEngineApplication->Shutdown();

		delete g_pClientApplication;
		delete g_pEngineApplication;

		TRACE_RECORD_STOP();
		TRACE_SESSION_END();

#if ION_ENABLE_TRACING
		DebugTracing::Shutdown();
#endif

		return 0;
	}
}
