#include "IonPCH.h"

#include "Core/Platform/Windows.h"

#include "WindowsWindow.h"
#include "WindowsApplication.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/Windows/OpenGLWindows.h"

#include "UserInterface/ImGui.h"

#pragma warning(disable:26812)

// ------------------------------
//        Windows Window
// ------------------------------

namespace Ion
{
	static const char* _windowNoInitMessage = "Cannot {0} before the window is initialized!";

	// Generic Window

	std::shared_ptr<GenericWindow> GenericWindow::Create()
	{
		return WindowsWindow::Create();
	}

	// Windows Window

	std::shared_ptr<WindowsWindow> WindowsWindow::Create()
	{
		return std::shared_ptr<WindowsWindow>(new WindowsWindow);
	}

	WindowsWindow::WindowsWindow() :
		m_WindowHandle(NULL),
		m_DeviceContext(NULL),
		m_RenderingContext(NULL)
	{
		WindowsWindowLogger.Info("Window has been created.");
	}

	WindowsWindow::~WindowsWindow() 
	{
		//DeleteRenderingContext();
		WindowsWindowLogger.Info("Window \"{}\" has been destroyed.", StringConverter::WStringToString(m_Title));
	}

	bool WindowsWindow::RegisterWindowClass(HINSTANCE hInstance, LPCWSTR className)
	{
		TRACE_FUNCTION();

		WindowsWindowLogger.Trace("Registering window class \"{}\"...", StringConverter::WStringToString(className));

		WNDCLASS wc = { };

		wc.style = CS_DBLCLKS | CS_OWNDC;
		wc.lpfnWndProc = WindowsApplication::WindowEventHandler;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = className;

		if (!RegisterClass(&wc))
		{
			MessageBox(NULL, L"Windows WNDCLASS Registration Failed!\nCannot create a window.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		WindowsWindowLogger.Info("Window class \"{}\" has been registered.", StringConverter::WStringToString(className));

		return true;
	}

	static const wchar* AppClassName = L"IonAppWindow";

	bool WindowsWindow::Initialize()
	{
		return Initialize(std::shared_ptr<GenericWindow>(nullptr));
	}

	bool WindowsWindow::Initialize(const std::shared_ptr<GenericWindow>& parentWindow)
	{
		TRACE_FUNCTION();

		WindowsWindowLogger.Trace("Initializing a window...");

		HINSTANCE hInstance = WindowsApplication::GetHInstance();

		if (!s_bClassRegistered)
		{
			if (!RegisterWindowClass(hInstance, AppClassName))
				return false;

			s_bClassRegistered = true;
		}

		UINT32 WindowStyleEx = 0;
		UINT32 WindowStyle = 0;

		WindowStyle |= WS_CAPTION;
		WindowStyle |= WS_MINIMIZEBOX;
		WindowStyle |= WS_MAXIMIZEBOX;
		WindowStyle |= WS_SYSMENU;
		WindowStyle |= WS_SIZEBOX;

		WindowStyleEx |= WS_EX_APPWINDOW;
		WindowStyleEx |= WS_EX_WINDOWEDGE;

		int32 windowWidth = 1280;
		int32 windowHeight = 720;

		RECT windowRect { };
		if (!AdjustWindowRectEx(&windowRect, WindowStyle, false, WindowStyleEx))
		{
			return false;
		}

		windowWidth += windowRect.right - windowRect.left;
		windowHeight += windowRect.bottom - windowRect.top;

		TRACE_BEGIN(0, "Win32::CreateWindowEx");

		m_WindowHandle = CreateWindowEx(
			WindowStyleEx,
			AppClassName,
			m_Title.c_str(),
			WindowStyle,

			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowWidth,
			windowHeight,

			NULL,
			NULL,
			hInstance,
			NULL
		);

		TRACE_END(0);

		if (m_WindowHandle == NULL)
		{
			MessageBox(NULL, L"Windows Window Creation Failed!\nCannot create a window.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		SetProp(m_WindowHandle, L"WinObj", this);

		m_DeviceContext = GetDC(m_WindowHandle);
		// @TODO: Move this
		//CreateRenderingContext(m_DeviceContext);

		WindowsWindowLogger.Info("Window \"{}\" has been created.", StringConverter::WStringToString(m_Title));

		return true;
	}

	void WindowsWindow::Show()
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Critical(_windowNoInitMessage, "show the window");
			return;
		}

		if (!m_bVisible)
		{
			m_bVisible = true;

			int32 showWindowCmd = SW_SHOW;
			ShowWindow(m_WindowHandle, showWindowCmd);
		}
	}

	void WindowsWindow::Hide()
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Critical(_windowNoInitMessage, "hide the window");
			return;
		}

		if (m_bVisible)
		{
			m_bVisible = false;

			ShowWindow(m_WindowHandle, SW_HIDE);
		}
	}

	void WindowsWindow::Maximize()
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Critical(_windowNoInitMessage, "maximize the window");
			return;
		}

		if (m_bVisible)
		{
			int32 showWindowCmd = SW_MAXIMIZE;
			ShowWindow(m_WindowHandle, showWindowCmd);
		}
	}

	void WindowsWindow::SetTitle(const WString& title)
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Critical(_windowNoInitMessage, "set the title");
			return;
		}

		m_Title = title;

		SetWindowText(m_WindowHandle, m_Title.c_str());
	}

	void WindowsWindow::SetEnabled(bool bEnabled)
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Critical(_windowNoInitMessage, "enable the window");
			return;
		}

		EnableWindow(m_WindowHandle, bEnabled);
	}

	void WindowsWindow::Resize()
	{

	}

	WindowDimensions WindowsWindow::GetDimensions() const
	{
		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Error(_windowNoInitMessage, "get window dimensions");
			return { 0, 0 };
		}
		
		RECT windowRect;
		GetClientRect(m_WindowHandle, &windowRect);

		int32 width = windowRect.right - windowRect.left;
		int32 height = windowRect.bottom - windowRect.top;
		return { width, height };
	}

	void WindowsWindow::EnableFullScreen(bool bFullscreen)
	{
		TRACE_FUNCTION();

		// Enable
		if (bFullscreen)
		{
			DWORD style = GetWindowLong(m_WindowHandle, GWL_STYLE);

			if (!m_bFullScreenMode)
			{
				WINDOWPLACEMENT windowPlacement { };
				windowPlacement.length = sizeof(WINDOWPLACEMENT);

				ionverify(GetWindowPlacement(m_WindowHandle, &windowPlacement));

				bool bMaximized = windowPlacement.showCmd == SW_MAXIMIZE;

				RECT clientRect { };
				GetClientRect(m_WindowHandle, &clientRect);

				m_WindowBeforeFullScreen = WindowDataBeforeFullScreen {
					windowPlacement,
					clientRect,
					style,
					bMaximized,
				};
			}

			MONITORINFO monitorInfo { };
			monitorInfo.cbSize = sizeof(MONITORINFO);

			HMONITOR hMonitor = MonitorFromWindow(m_WindowHandle, MONITOR_DEFAULTTOPRIMARY);
			ionverify(GetMonitorInfo(hMonitor, &monitorInfo));

			// This here changes to "real" fullscreen without any input lag.
			DEVMODE devMode { };
			ionverify(EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode));
			ionverify(ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);

			ionverify(SetWindowLong(m_WindowHandle, GWL_STYLE, (style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP));

			int32 x = monitorInfo.rcMonitor.left;
			int32 y = monitorInfo.rcMonitor.top;
			int32 width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			int32 height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
			ionverify(SetWindowPos(m_WindowHandle, HWND_TOP, x, y, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED));

			m_bFullScreenMode = true;

			RHI::Get()->ChangeDisplayMode(GetRHIData(), EWindowDisplayMode::FullScreen, width, height);

			WindowChangeDisplayModeEvent event((void*)m_WindowHandle, EDisplayMode::FullScreen, width, height, EVENT_DEBUG_NAME(m_Title));
			WindowsApplication::PostEvent(event);
		}
		// Disable
		else
		{
			ionverify(SetWindowLong(m_WindowHandle, GWL_STYLE, m_WindowBeforeFullScreen.Style));

			RECT& clientRect = m_WindowBeforeFullScreen.ClientRect;
			int32 clientWidth = clientRect.right - clientRect.left;
			int32 clientHeight = clientRect.bottom - clientRect.top;

			RHI::Get()->ChangeDisplayMode(GetRHIData(), EWindowDisplayMode::Windowed, clientWidth, clientHeight);

			if (m_bFullScreenMode)
			{
				RECT& windowRect = m_WindowBeforeFullScreen.WindowPlacement.rcNormalPosition;
				int32 x = windowRect.left;
				int32 y = windowRect.top;
				int32 width = windowRect.right - windowRect.left;
				int32 height = windowRect.bottom - windowRect.top;

				if (m_WindowBeforeFullScreen.bMaximized)
				{
					ionverify(ShowWindow(m_WindowHandle, SW_MAXIMIZE));
				}
				else
				{
					ionverify(SetWindowPos(m_WindowHandle, HWND_NOTOPMOST, x, y, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED));
				}
			}

			m_bFullScreenMode = false;

			WindowChangeDisplayModeEvent event((void*)m_WindowHandle, EDisplayMode::Windowed, clientWidth, clientHeight, EVENT_DEBUG_NAME(m_Title));
			WindowsApplication::PostEvent(event);
		}
	}

	bool WindowsWindow::IsFullScreenEnabled() const
	{
		return m_bFullScreenMode;
	}

	bool WindowsWindow::IsInFocus() const
	{
		return GetFocus() == m_WindowHandle;
	}

	void WindowsWindow::ClipCursor()
	{
		RECT clientRect { };
		GetClientRect(m_WindowHandle, &clientRect);
		ClientToScreen(m_WindowHandle, (POINT*)&clientRect.left);
		ClientToScreen(m_WindowHandle, (POINT*)&clientRect.right);
		::ClipCursor(&clientRect);
		m_bCursorLocked = true;
	}

	void WindowsWindow::LockCursor(IVector2 position)
	{
		ClientToScreen(m_WindowHandle, (POINT*)&position);
		RECT rect = { position.x, position.y, position.x + 1, position.y + 1 };
		::ClipCursor(&rect);
		m_bCursorLocked = true;
	}

	void WindowsWindow::LockCursor()
	{
		POINT cursorPos { };
		GetCursorPos(&cursorPos);
		RECT rect = { cursorPos.x, cursorPos.y, cursorPos.x + 1, cursorPos.y + 1 };
		::ClipCursor(&rect);
		m_bCursorLocked = true;
	}

	void WindowsWindow::LockCursor(bool bLock)
	{
		if (bLock)
			LockCursor();
		else
			UnlockCursor();
	}

	void WindowsWindow::UnlockCursor()
	{
		::ClipCursor(nullptr);
		m_bCursorLocked = false;
	}

	void WindowsWindow::ShowCursor(bool bShow)
	{
		CURSORINFO info { };
		info.cbSize = sizeof(CURSORINFO);
		GetCursorInfo(&info);

		// This is to avoid incrementing / decrementing the display count too much
		if ((info.flags & CURSOR_SHOWING) != bShow)
		{
			::ShowCursor(bShow);
			m_bCursorShown = bShow;
		}
	}

	//HGLRC WindowsWindow::CreateRenderingContext(HDC deviceContext, HGLRC parentContext)
	//{
	//	TRACE_FUNCTION();

	//	if (m_WindowHandle == NULL)
	//	{
	//		WindowsWindowLogger.Critical(_windowNoInitMessage, "create OpenGL rendering context");
	//		return NULL;
	//	}

	//	// Setup Rendering Context
	//	m_RenderingContext = OpenGLWindows::CreateGLContext(deviceContext, parentContext);
	//	return m_RenderingContext;
	//}

	//void WindowsWindow::DeleteRenderingContext()
	//{
	//	if (m_RenderingContext != NULL)
	//	{
	//		wglDeleteContext(m_RenderingContext);
	//	}
	//}

	//void WindowsWindow::MakeRenderingContextCurrent()
	//{
	//	// @TODO: Move this whole "MakeContextCurrent" thing into RenderAPI specific files
	//	// It shouldn't be tied to the window at all

	//	OpenGLWindows::MakeContextCurrent(m_DeviceContext, m_RenderingContext);
	//}

	void WindowsWindow::SwapBuffers()
	{
		TRACE_FUNCTION();

		// @TODO: Create some sort of OpenGL State manager, so useless calls are not made.
		MakeRenderingContextCurrent();
		::SwapBuffers(m_DeviceContext);
	}

	void* WindowsWindow::GetNativeHandle() const
	{
		return (void*)m_WindowHandle;
	}

	HDC WindowsWindow::GetDeviceContext() const
	{
		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Critical(_windowNoInitMessage, "get the Device Context");
			return NULL;
		}

		return m_DeviceContext;
	}

	HGLRC WindowsWindow::GetRenderingContext() const
	{
		if (m_WindowHandle == NULL)
		{
			WindowsWindowLogger.Critical(_windowNoInitMessage, "get the Rendering Context");
			return NULL;
		}

		return m_RenderingContext;
	}

	bool WindowsWindow::s_bClassRegistered = false;
}
