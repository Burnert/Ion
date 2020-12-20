#pragma once

#ifdef ION_PLATFORM_WINDOWS

//extern Ion::Application* Ion::CreateApplication();

int main(int argc, char** argv)
{
	Ion::Application* application = Ion::CreateApplication();
	application->Run();
	delete application;
}

#endif