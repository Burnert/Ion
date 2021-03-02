#include "IonPCH.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "Core/Platform/Windows/WindowsInput.h"

#ifdef ION_PLATFORM_WINDOWS

#include "OpenGL/Windows/OpenGLWindows.h"

#endif

DECLARE_PERFORMANCE_COUNTER(RendererAPI_InitTime, "RendererAPI Init Time", "Init");

namespace Ion
{
	WindowsApplication* WindowsApplication::Get()
	{
		return static_cast<WindowsApplication*>(Application::Get());
	}

	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		m_HInstance = hInstance;

		Init();
	}

	HGLRC WindowsApplication::CreateRenderingContext(HDC hdc)
	{
		return OpenGLWindows::CreateGLContext(hdc);
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

	void WindowsApplication::InitRendererAPI()
	{
		SCOPED_PERFORMANCE_COUNTER(RendererAPI_InitTime);

		// @TODO: Add renderer switch here
		OpenGLWindows::InitLoader();
	}

	HINSTANCE WindowsApplication::m_HInstance;
}
