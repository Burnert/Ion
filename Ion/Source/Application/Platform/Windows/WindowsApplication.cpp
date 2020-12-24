#include "IonPCH.h"

#include "WindowsApplication.h"

namespace Ion
{
	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		m_HInstance = hInstance;
		Init();
	}

	HINSTANCE WindowsApplication::m_HInstance;
}