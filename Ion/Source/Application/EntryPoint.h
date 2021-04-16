#pragma once

#ifdef ION_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	Ion::Application* application = Ion::CreateApplication();
	application->Start();

	TRACE_SESSION_BEGIN("Shutdown");
	delete application;
	TRACE_SESSION_END();

#ifdef ION_DEBUG
	ION_LOG_DEBUG("Press Enter to close.");
	getchar();
#endif

	return 0;
}

#else

#error Only Windows supported.

#endif
