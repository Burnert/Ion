#include "IonPCH.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "Core/Platform/Windows/WindowsInput.h"

namespace Ion
{
	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		m_HInstance = hInstance;
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
	}

	void WindowsApplication::Update(float DeltaTime)
	{
		std::shared_ptr window = std::static_pointer_cast<WindowsWindow>(GetApplicationWindow());
		window->Update_Application();
	}

	HINSTANCE WindowsApplication::m_HInstance;
}
