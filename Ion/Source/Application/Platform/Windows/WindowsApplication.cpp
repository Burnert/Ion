#include "IonPCH.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "Core/Platform/Windows/WindowsInput.h"

#include "RenderAPI/RenderAPI.h"

#include "RenderAPI/OpenGL/Windows/OpenGLWindows.h"

namespace Ion
{
	WindowsApplication* WindowsApplication::Get()
	{
		return static_cast<WindowsApplication*>(Application::Get());
	}

	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		m_HInstance = hInstance;

		LARGE_INTEGER largeInteger { 0 };
		VERIFY(QueryPerformanceFrequency(&largeInteger));
		s_PerformanceFrequency = (float)largeInteger.QuadPart;

		Init();
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

	void WindowsApplication::HandleEvent(Event& event)
	{
		Application::HandleEvent(event);
	}

	void WindowsApplication::DispatchEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		// Handle close event in application
		dispatcher.Dispatch<WindowCloseEvent>(
			[this](WindowCloseEvent& event)
			{
				DestroyWindow((HWND)event.GetWindowHandle());
				return false;
			});

		Application::DispatchEvent(event);
	}

	float WindowsApplication::CalculateFrameTime()
	{
		// @TODO: Would be nice if it were pausable

		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);

		if (s_LastFrameTime == 0.0f)
		{
			s_LastFrameTime = (float)time.QuadPart;
		}

		float difference;
		difference = time.QuadPart - s_LastFrameTime;

		s_LastFrameTime = (float)time.QuadPart;

		return difference / s_PerformanceFrequency;
	}

	HINSTANCE WindowsApplication::m_HInstance;

	float WindowsApplication::s_PerformanceFrequency;
	float WindowsApplication::s_LastFrameTime { 0 };
}
