#include "IonPCH.h"

#include "WindowsWindow.h"
#include "Log/Logger.h"

namespace Ion
{
	std::shared_ptr<GenericWindow> GenericWindow::Create()
	{
		return Ion::WindowsWindow::Create();
	}

	std::shared_ptr<WindowsWindow> WindowsWindow::Create()
	{
		ION_LOG_ENGINE_TRACE("Creating Windows window.");
		return std::shared_ptr<WindowsWindow>(new WindowsWindow);
	}

	WindowsWindow::WindowsWindow() :
		m_HWnd(NULL),
		m_HDC(NULL)
	{
	}

	WindowsWindow::~WindowsWindow()
	{
	}

	void WindowsWindow::Initialize()
	{

	}

	void WindowsWindow::SetTitle(WCStr title)
	{
	}

	void WindowsWindow::Resize()
	{
	}

	bool WindowsWindow::GetDimensions(int& width, int& height) const
	{
		return false;
	}


}

