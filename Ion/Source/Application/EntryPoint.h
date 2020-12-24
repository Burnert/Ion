#pragma once

#ifdef ION_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	Ion::Application* application = Ion::CreateApplication();
	application->InitWindows(hInstance);
	delete application;
	return 0;
}

#endif
