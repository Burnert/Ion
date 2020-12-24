#pragma once

#ifdef ION_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	Ion::WindowsApplication* application = dynamic_cast<Ion::WindowsApplication*>(Ion::CreateApplication());
	application->InitWindows(hInstance);
	delete application;
	return 0;
}

#else

#error Only Windows supported.

#endif
