#include "IonPCH.h"

#include "WindowsApplication.h"

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

	HINSTANCE WindowsApplication::m_HInstance;
}
