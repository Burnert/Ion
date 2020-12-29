#include "IonPCH.h"

#include "WindowsWindow.h"
#include "WindowsApplication.h"

#include "Core/Event/Event.h"
#include "Core/Event/WindowEvent.h"


#include "Log/Logger.h"

namespace Ion
{
	static const CStr _windowNoInitMessage = "Cannot {0} before the window is initialized!";

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

		// Handle destroy event first

		if (uMsg == WM_DESTROY)
		{
			PostQuitMessage(0);

			DeferredEventPtr event = Event::CreateDeferredEvent<WindowCloseEvent>((ullong)hWnd);
			windowRef.m_EventCallback(*event);

			return 0;
		}

		switch (uMsg)
		{
			// Deferred events

			case WM_SETFOCUS:
			{
				DeferredEventPtr event = Event::CreateDeferredEvent<WindowFocusEvent>((ullong)hWnd);
				windowRef.m_EventCallback(*event);

				return 0;
			}

			case WM_KILLFOCUS:
			{
				DeferredEventPtr event = Event::CreateDeferredEvent<WindowLostFocusEvent>((ullong)hWnd);
				windowRef.m_EventCallback(*event);

				return 0;
			}

			// Non-Deferred events

			case WM_MOVE:
			{
				int xPos = LOWORD(lParam);
				int yPos = HIWORD(lParam);
				auto event = WindowMovedEvent((ullong)hWnd, xPos, yPos);
				windowRef.m_EventCallback(event);

				return 0;
			}

			case WM_SIZE:
			{
				int width  = LOWORD(lParam);
				int height = HIWORD(lParam);
				auto event = WindowResizeEvent((ullong)hWnd, width, height);
				windowRef.m_EventCallback(event);

				return 0;
			}
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
		WindowStyle |= WS_SIZEBOX;

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
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "show the window");
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
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "hide the window");
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
		if (m_HWnd == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "set the title");
			return;
		}

		m_Title = title;

		SetWindowText(m_HWnd, m_Title.c_str());
	}

	void WindowsWindow::SetEnabled(bool bEnabled)
	{
		if (m_HWnd == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "enable the window");
			return;
		}

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
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "get the HDC");
			return NULL;
		}

		return GetDC(m_HWnd);
	}
}

