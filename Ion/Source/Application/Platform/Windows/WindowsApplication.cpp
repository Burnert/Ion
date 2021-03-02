#include "IonPCH.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "Core/Platform/Windows/WindowsInput.h"

namespace Ion
{
	WindowsApplication* WindowsApplication::Get()
	{
		return static_cast<WindowsApplication*>(Application::Get());
	}

	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		m_HInstance = hInstance;
		m_OpenGLModule = LoadLibrary(TEXT("opengl32.dll"));
		Init();
	}

	void* WindowsApplication::GetProcessAddress(const char* name)
	{
		void* address = wglGetProcAddress(name);
		if (address)
			return address;

		return GetProcAddress(m_OpenGLModule, name);
	}

	void WindowsApplication::PollEvents()
	{
		MSG message;

		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		std::static_pointer_cast<WindowsWindow>(GetApplicationWindow())->PollEvents_Application();
	}

	void WindowsApplication::Update(float DeltaTime)
	{
		Application::Update(DeltaTime);
	}

	void WindowsApplication::Render()
	{
		Application::Render();
	}

	void WindowsApplication::OnEvent(Event& event)
	{
		Application::OnEvent(event);
	}

	void WindowsApplication::DispatchEvent(Event& event)
	{
		Application::DispatchEvent(event);
	}

	HINSTANCE WindowsApplication::m_HInstance;
	HMODULE WindowsApplication::m_OpenGLModule;
}
