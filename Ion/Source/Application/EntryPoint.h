#pragma once

namespace Ion
{
	Application* InstantiateApplication();
}

#ifdef ION_PLATFORM_WINDOWS

static int32 MainShared()
{
	using namespace Ion;
#if ION_ENABLE_TRACING
	DebugTracing::Init();
#endif
	DebugTimer::InitPlatform();

	Application* application = InstantiateApplication();
	application->Start();

	TRACE_SESSION_BEGIN("Shutdown");
	TRACE_RECORD_START();
	delete application;
	TRACE_RECORD_STOP();
	TRACE_SESSION_END();

#ifdef ION_DEBUG
#ifndef DISABLE_PAUSE_ON_EXIT
	ION_LOG_DEBUG("Press Enter to close.");
	(void)getchar();
#endif
#endif

#if ION_ENABLE_TRACING
	DebugTracing::Shutdown();
#endif

	return 0;
}

#ifndef UNICODE

int32 main(int32 argc, char* argv[])
{
	Ion::ParseCommandLineArgs(argc, argv);
	MainShared();
}

#else

int32 wmain(int32 argc, wchar* argv[])
{
	Ion::ParseCommandLineArgs(argc, argv);
	MainShared();
}

#endif

#else

#error Only Windows supported.

#endif
