#pragma once

#ifdef ION_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	Ion::Application* application = Ion::CreateApplication();
	application->Run();
	delete application;
	return 0;
}

#endif
