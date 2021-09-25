#pragma once

#ifdef ION_PLATFORM_WINDOWS

static int MainShared()
{
	Ion::Application* application = Ion::CreateApplication();
	application->Start();

	TRACE_SESSION_BEGIN("Shutdown");
	TRACE_RECORD_START();
	delete application;
	TRACE_RECORD_STOP();
	TRACE_SESSION_END();

#ifdef ION_DEBUG
	ION_LOG_DEBUG("Press Enter to close.");
	getchar();
#endif

	return 0;
}

#ifndef UNICODE

int main(int argc, char* argv[])
{
	Ion::ParseCommandLineArgs(argc, argv);
	MainShared();
}

#else

int wmain(int argc, wchar* argv[])
{
	Ion::ParseCommandLineArgs(argc, argv);
	MainShared();
}

#endif

#else

#error Only Windows supported.

#endif
