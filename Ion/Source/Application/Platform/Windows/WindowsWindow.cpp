#include "IonPCH.h"

#include "WindowsWindow.h"
#include "WindowsApplication.h"

#include "Event/Event.h"
#include "Event/WindowEvent.h"


#include "Log/Logger.h"

namespace Ion
{
	static const CStr _windowNotCreatedMessage = "Cannot {0} before the window is initialized!";

	// Generic Window

	std::shared_ptr<GenericWindow> GenericWindow::Create()
	{
		return Ion::WindowsWindow::Create();
	}

	// Windows Window

	std::shared_ptr<WindowsWindow> WindowsWindow::Create()
	{
		ION_LOG_ENGINE_TRACE("Creating Windows window.");
		return std::shared_ptr<WindowsWindow>(new WindowsWindow);
	}

	WindowsWindow::WindowsWindow() :
		m_HWnd(NULL)
	{}

	LRESULT WindowsWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		WindowsWindow& windowRef = *(WindowsWindow*)GetProp(hWnd, L"WinObj");

		if (uMsg == WM_DESTROY)
		{
			PostQuitMessage(0);

			auto event = new WindowCloseEvent((int)hWnd);
			windowRef.m_EventCallback(event->MakeShared());

			return 0;
		}

		switch (uMsg)
		{

		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	WindowsWindow::~WindowsWindow() {}

	bool WindowsWindow::RegisterWindowClass(HINSTANCE hInstance, LPCWSTR className)
	{
		WNDCLASS wc = { };

		if (!m_bRegistered)
		{
			wc.style = CS_DBLCLKS;
			wc.lpfnWndProc = WindowsWindow::WindowProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = hInstance;
			wc.hIcon = NULL;
			wc.hCursor = NULL;
			wc.hbrBackground = NULL;
			wc.lpszMenuName = NULL;
			wc.lpszClassName = className;

			if (!RegisterClass(&wc))
			{
				MessageBox(NULL, L"Windows WNDCLASS Registration Failed!\nCannot create a window.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
				return false;
			}

			m_bRegistered = true;
			return true;
		}
		return false;
	}

	const wchar* AppClassName = L"IonAppWindow";

	bool WindowsWindow::Initialize()
	{
		HINSTANCE hInstance = WindowsApplication::GetHInstance();

		if (!RegisterWindowClass(hInstance, AppClassName)) 
			return false;

		UINT32 WindowStyleEx = 0;
		UINT32 WindowStyle = 0;

		WindowStyle |= WS_CAPTION;
		WindowStyle |= WS_MINIMIZEBOX;
		WindowStyle |= WS_SYSMENU;

		WindowStyleEx |= WS_EX_APPWINDOW;
		WindowStyleEx |= WS_EX_WINDOWEDGE;

		int desiredWidth = 1280;
		int desiredHeight = 720;

		m_HWnd = CreateWindowEx(
			WindowStyleEx,
			AppClassName,
			m_Title.c_str(),
			WindowStyle,

			CW_USEDEFAULT,
			CW_USEDEFAULT,
			desiredWidth,
			desiredHeight,

			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (m_HWnd == NULL)
		{
			MessageBox(NULL, L"Windows Window Creation Failed!\nCannot create a window.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		SetProp(m_HWnd, L"WinObj", this);

		return true;
	}

	void WindowsWindow::Show()
	{
		if (m_HWnd == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNotCreatedMessage, "show the window");
			return;
		}

		if (!m_bVisible)
		{
			m_bVisible = true;

			int showWindowCmd = SW_SHOW;
			ShowWindow(m_HWnd, showWindowCmd);
		}
	}

	void WindowsWindow::Hide()
	{
		if (m_HWnd == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNotCreatedMessage, "hide the window");
			return;
		}

		if (m_bVisible)
		{
			m_bVisible = false;

			ShowWindow(m_HWnd, SW_HIDE);
		}
	}

	void WindowsWindow::SetTitle(const WCStr title)
	{
		if (m_HWnd != NULL)
		{
			m_Title = title;

			SetWindowText(m_HWnd, m_Title.c_str());
		}
	}

	void WindowsWindow::SetEnabled(bool bEnabled)
	{
		EnableWindow(m_HWnd, bEnabled);
	}

	void WindowsWindow::Resize()
	{
	}

	bool WindowsWindow::GetDimensions(int& width, int& height) const
	{

		return false;
	}

	HDC WindowsWindow::GetHDC() const
	{
		if (m_HWnd == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNotCreatedMessage, "get the HDC");
			return NULL;
		}

		return GetDC(m_HWnd);
	}
}

