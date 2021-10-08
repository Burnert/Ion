#include "IonPCH.h"

#include "WindowsWindow.h"
#include "WindowsApplication.h"

#include "Core/Event/Event.h"
#include "Core/Event/WindowEvent.h"
#include "Core/Event/InputEvent.h"
#include "Core/Input/Input.h"
#include "Core/Logging/Logger.h"

#include "Core/Platform/PlatformCore.h"

#include "RenderAPI/OpenGL/Windows/OpenGLWindows.h"

#include "UserInterface/ImGui.h"

#pragma warning(disable:26812)

// ------------------------------
//        Windows Window
// ------------------------------

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Ion
{
	static const char* _windowNoInitMessage = "Cannot {0} before the window is initialized!";

	// Post events directly to Application
	// This is needed for compile time event dispatch
	template<typename T>
	inline void PostEvent(const T& event)
	{
		Application::Get()->PostEvent(event);
	}
	// Post events directly to Application
	// The type is needed for copying the events to the event queue
	template<typename T>
	inline void PostDeferredEvent(const T& event)
	{
		Application::Get()->PostDeferredEvent(event);
	}

	// Generic Window

	TShared<GenericWindow> GenericWindow::Create()
	{
		return Ion::WindowsWindow::Create();
	}

	// Windows Window

	TShared<WindowsWindow> WindowsWindow::Create()
	{
		ION_LOG_ENGINE_TRACE("Creating Windows window.");
		return TShared<WindowsWindow>(new WindowsWindow);
	}

	WindowsWindow::WindowsWindow() :
		m_WindowHandle(NULL),
		m_DeviceContext(NULL),
		m_RenderingContext(NULL)
	{ }

	void WindowsWindow::PollEvents_Application()
	{
		TRACE_FUNCTION();

		// Shift hack:
		if (m_bBothShiftsPressed)
		{
			uint8 keyState = 0;
			keyState |= (GetAsyncKeyState(VK_LSHIFT) & 0x8000) >> 14;
			keyState |= (GetAsyncKeyState(VK_RSHIFT) & 0x8000) >> 15;
			if (keyState != 0x3)
			{
				uint32 actualKeyCode;
				// Left shift held, so right shift released and vice-versa
				if (keyState == 0x2)
					actualKeyCode = Key::RShift;
				else 
					actualKeyCode = Key::LShift;
				m_bBothShiftsPressed = false;

				KeyReleasedEvent event(Key::Shift, actualKeyCode);
				PostEvent(event);
			}
		}
	}

	LRESULT WindowsWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		TRACE_FUNCTION();

		TRACE_BEGIN(0, "WindowsWindow - ImGui_ImplWin32_WndProcHandler");
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return 1;
		TRACE_END(0);

		// Read ImGui input first and decide what to do
		// @TODO: This is a quick fix so do something better in the future
		if (ImGui::GetCurrentContext())
		{
			ImGuiIO& imGuiIO = ImGui::GetIO();
			if (imGuiIO.WantCaptureKeyboard)
			{
				if (IsAnyOf(uMsg,
					WM_KEYDOWN,
					WM_SYSKEYDOWN,
					WM_KEYUP,
					WM_KEYUP,
					WM_SYSKEYUP))
				{
					return 0;
				}
			}
			if (imGuiIO.WantCaptureMouse)
			{
				if (IsAnyOf(uMsg, 
					WM_LBUTTONUP,
					WM_RBUTTONUP,
					WM_MBUTTONUP,
					WM_XBUTTONUP,
					WM_LBUTTONDOWN,
					WM_RBUTTONDOWN,
					WM_MBUTTONDOWN,
					WM_XBUTTONDOWN,
					WM_LBUTTONDBLCLK,
					WM_RBUTTONDBLCLK,
					WM_MBUTTONDBLCLK,
					WM_XBUTTONDBLCLK,
					WM_MOUSEWHEEL,
					WM_MOUSEMOVE,
					WM_INPUT))
				{
					return 0;
				}
			}
		}

		WindowsWindow& windowRef = *(WindowsWindow*)GetProp(hWnd, L"WinObj");

		// --------------------------------------
		// Convert Windows messages to Ion Events
		// --------------------------------------

		switch (uMsg)
		{
			// -----------------------------------------
			//  Deferred events 
			// -----------------------------------------

			case WM_SETFOCUS:
			{
				WindowFocusEvent event((uint64)hWnd);
				PostDeferredEvent(event);

				return 0;
			}

			case WM_KILLFOCUS:
			{
				WindowLostFocusEvent event((uint64)hWnd);
				PostDeferredEvent(event);

				return 0;
			}

			case WM_CLOSE:
			{
				WindowCloseEvent event((uint64)hWnd);
				PostDeferredEvent(event);

				return 0;
			}

			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}

			// -----------------------------------------
			//  Non-Deferred events
			// -----------------------------------------

			case WM_MOVE:
			{
				POINTS pos = MAKEPOINTS(lParam);
				int32 xPos = pos.x;
				int32 yPos = pos.y;
				WindowMovedEvent event((uint64)hWnd, xPos, yPos);
				PostEvent(event);

				return 0;
			}

			case WM_SIZE:
			{
				int32 width  = LOWORD(lParam);
				int32 height = HIWORD(lParam);
				WindowResizeEvent event((uint64)hWnd, width, height);
				PostEvent(event);

				return 0;
			}

			// Keyboard events

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				uint32 keyCode       = (uint32)wParam;
				uint32 actualKeyCode = keyCode;
				bool bState        = !(HIWORD(lParam) & KF_UP);
				bool bExtendedKey  = (HIWORD(lParam) & KF_EXTENDED);

				if (keyCode == VK_CONTROL)
				{
					// HACK:
					// Right Alt key press sends a Left Ctrl key message
					// and then the actual Alt one, but we don't need that,
					// so if this sequence gets detected the Left Ctrl
					// message has to be discarded.
					if (!bExtendedKey)
					{
						MSG nextMsg;
						if (PeekMessage(&nextMsg, NULL, 0, 0, PM_NOREMOVE))
						{
							// HACK: (of a hack)
							// I don't know what the fuck this is, but Windows
							// sometimes decides to send a 0x60 message (which isn't
							// documented anywhere btw) before the RAlt one, so I have to
							// check it here so it doesn't lock the LControl key.
							if (nextMsg.message == 0x60)
								break;

							if (nextMsg.message == WM_KEYDOWN ||
								nextMsg.message == WM_SYSKEYDOWN ||
								nextMsg.message == WM_KEYUP ||
								nextMsg.message == WM_SYSKEYUP)
							{
								if (nextMsg.wParam == VK_MENU &&
									(HIWORD(nextMsg.lParam) & KF_EXTENDED))
									// Discard the LCtrl right before the RAlt
									break;
							}
						}
						actualKeyCode = VK_LCONTROL;
					}
					else
					{
						actualKeyCode = VK_RCONTROL;
					}
				}
				else if (keyCode == VK_SHIFT)
				{
					// HACK:
					// When releasing a Shift key when both right and left
					// are pressed, Windows doesn't send a message. If both
					// keys are down we start checking each frame if one
					// of them was released and fire a proper event.
					//
					// Rest of the code in PollEvents_Application()
					if ((GetKeyState(VK_LSHIFT) & 0x8000) &&
						(GetKeyState(VK_RSHIFT) & 0x8000))
					{
						m_bBothShiftsPressed = true;
					}

					// Note:
					// Distinguishing between left and right Shift keys
					// is a bit diffrent from the other keys.
					uint8 scanCode = LOBYTE(HIWORD(lParam));

					if (scanCode == 0x36)
						actualKeyCode = VK_RSHIFT;
					else
						actualKeyCode = VK_LSHIFT;
				}
				else if (keyCode == VK_MENU)
				{
					if (bExtendedKey)
						actualKeyCode = VK_RMENU;
					else
						actualKeyCode = VK_LMENU;
				}

				// Translate Windows keycodes to internal ones
				WindowsInputManager::TranslateWindowsKeyCode(&keyCode);
				WindowsInputManager::TranslateWindowsKeyCode(&actualKeyCode);

				// HACK:
				// Windows doesn't have a separate keycode for keypad Enter button,
				// but the key is extended in this case. Therefore, if the keycode
				// is an extended Enter key, change the internal (translated)
				// keycode to KP_Enter.
				if (keyCode == Key::Enter && bExtendedKey)
				{
					keyCode = Key::KP_Enter;
					actualKeyCode = Key::KP_Enter;
				}

				// Down: true, Up: false
				if (bState)
				{
					// If the key is already pressed it means it was repeated
					bool bRepeated = InputManager::IsKeyPressed((KeyCode)actualKeyCode);

					if (bRepeated)
					{
						KeyRepeatedEvent event(keyCode, actualKeyCode);
						PostEvent(event);
					}
					else
					{
						KeyPressedEvent event(keyCode, actualKeyCode);
						PostEvent(event);
					}
				}
				else
				{
					// HACK:
					// Print screen key doesn't send a key down message,
					// so upon release send PressedEvent before the ReleaseEvent.
					// At this point keycode variables are already translated.
					if (keyCode == Key::PrintScr)
					{
						KeyPressedEvent printScreenPressed(keyCode, actualKeyCode);
						PostEvent(printScreenPressed);
					}

					KeyReleasedEvent event(keyCode, actualKeyCode);
					PostEvent(event);
				}

				return 0;
			}

			// Mouse events

			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_XBUTTONUP:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_XBUTTONDOWN:
			{
				uint32 button = MouseButtonFromMessage(uMsg, wParam);;
				bool bState =
					uMsg == WM_LBUTTONDOWN ||
					uMsg == WM_RBUTTONDOWN ||
					uMsg == WM_MBUTTONDOWN ||
					uMsg == WM_XBUTTONDOWN;

				// Down: true, Up: false
				if (bState)
				{
					MouseButtonPressedEvent event(button);
					PostEvent(event);
				}
				else
				{
					MouseButtonReleasedEvent event(button);
					PostEvent(event);
				}

				return 0;
			}

			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDBLCLK:
			{
				uint32 button = MouseButtonFromMessage(uMsg, wParam);

				MouseButtonPressedEvent pressedEvent(button);
				PostEvent(pressedEvent);

				MouseDoubleClickEvent doubleClickEvent(button);
				PostEvent(doubleClickEvent);

				return 0;
			}

			case WM_MOUSEWHEEL:
			{
				float delta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
				MouseScrolledEvent event(delta);
				PostEvent(event);

				return 0;
			}

			case WM_MOUSEMOVE:
			{
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);

				int32 xPos = GET_X_LPARAM(lParam);
				int32 yPos = GET_Y_LPARAM(lParam);

				float xNormalised = (float)xPos / clientRect.right;
				float yNormalised = (float)yPos / clientRect.bottom;

				MouseMovedEvent event(xNormalised, yNormalised);
				PostEvent(event);

				return 0;
			}

			case WM_SYSCOMMAND:
			{
				// If system menu is activated by LAlt key
				// return 0 so Windows doesn't beep, because
				// there is actually no menu.
				if (wParam == SC_KEYMENU && HIWORD(lParam) <= 0)
					return 0;

				break;
			}

			// Raw Input

			case WM_INPUT:
			{
				// Don't bother if RawInput is not enabled.
				if (!InputManager::IsRawInputEnabled())
					break;

				uint32 size;
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
				uint8* buffer = new uint8[size];
				if (buffer == NULL)
					return 0;

				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size)
				{
					LOG_WARN("GetRawInputData does not return correct size!");
					return 0;
				}

				RAWINPUT* rawInput = (RAWINPUT*)buffer;

				// Mouse Input
				if (rawInput->header.dwType == RIM_TYPEMOUSE)
				{
					RAWMOUSE* data = &rawInput->data.mouse;

					if (InputManager::GetMouseInputType() == MouseInputType::RawInput)
					{
						// Note: Literally every input action can be packed
						// as a single event so everything here has to be checked
						// each time to make sure not to miss any.

						// Post RawInputMouseMoveEvent if the mouse actually moved on this message
						if (data->lLastX != 0 ||
							data->lLastY != 0)
						{
							RawInputMouseMovedEvent event((float)data->lLastX, (float)data->lLastY);
							PostEvent(event);
						}

						// Mouse Scrolled Event
						if (data->usButtonFlags & RI_MOUSE_WHEEL)
						{
							RawInputMouseScrolledEvent event((float)(int16)data->usButtonData);
							PostEvent(event);
						}

						// Mouse Button Pressed Event
						// 0x0155 - mouse pressed button flags combined
						if (data->usButtonFlags & 0x0155)
						{
							// There can be multiple presses in one message so I have
							// to loop through all of them here so everything gets
							// sent as an event.
							for (uint32 flag = Bitflag(0); flag != Bitflag(10); flag <<= 2)
							{
								uint32 buttonFlag = data->usButtonFlags & flag;
								if (buttonFlag)
								{
									uint32 button;
									switch (buttonFlag)
									{
									case RI_MOUSE_LEFT_BUTTON_DOWN:     button = Mouse::Left;      break;
									case RI_MOUSE_RIGHT_BUTTON_DOWN:    button = Mouse::Right;     break;
									case RI_MOUSE_MIDDLE_BUTTON_DOWN:   button = Mouse::Middle;    break;
									case RI_MOUSE_BUTTON_4_DOWN:        button = Mouse::Button4;   break;
									case RI_MOUSE_BUTTON_5_DOWN:        button = Mouse::Button5;   break;
									}

									RawInputMouseButtonPressedEvent event(button);
									PostEvent(event);
								}
							}
						}

						// Mouse Button Released Event
						// 0x02AA - mouse released button flags combined
						if (data->usButtonFlags & 0x02AA)
						{
							// Similar loop for button releases
							for (uint32 flag = Bitflag(1); flag != Bitflag(11); flag <<= 2)
							{
								uint32 buttonFlag = data->usButtonFlags & flag;
								if (buttonFlag)
								{
									uint32 button;
									switch (buttonFlag)
									{
									case RI_MOUSE_LEFT_BUTTON_UP:     button = Mouse::Left;      break;
									case RI_MOUSE_RIGHT_BUTTON_UP:    button = Mouse::Right;     break;
									case RI_MOUSE_MIDDLE_BUTTON_UP:   button = Mouse::Middle;    break;
									case RI_MOUSE_BUTTON_4_UP:        button = Mouse::Button4;   break;
									case RI_MOUSE_BUTTON_5_UP:        button = Mouse::Button5;   break;
									}

									RawInputMouseButtonReleasedEvent event(button);
									PostEvent(event);
								}
							}
						}
					}
				}

				delete[] buffer;
				return 0;
			}
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	MouseButton WindowsWindow::MouseButtonFromMessage(UINT uMsg, WPARAM wParam)
	{
		if (uMsg <= WM_LBUTTONDBLCLK)
			return Mouse::Left;
		else if (uMsg <= WM_RBUTTONDBLCLK)
			return Mouse::Right;
		else if (uMsg <= WM_MBUTTONDBLCLK)
			return Mouse::Middle;
		else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
			return Mouse::Button4;
		else
			return Mouse::Button5;
	}

	WindowsWindow::~WindowsWindow() 
	{
		DeleteRenderingContext();
	}

	bool WindowsWindow::RegisterWindowClass(HINSTANCE hInstance, LPCWSTR className)
	{
		TRACE_FUNCTION();

		WNDCLASS wc = { };

		wc.style = CS_DBLCLKS | CS_OWNDC;
		wc.lpfnWndProc = WindowsWindow::WindowProc;
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

		return true;
	}

	static const wchar* AppClassName = L"IonAppWindow";

	bool WindowsWindow::Initialize()
	{
		return Initialize(TShared<GenericWindow>(nullptr));
	}

	bool WindowsWindow::Initialize(const TShared<GenericWindow>& parentWindow)
	{
		TRACE_FUNCTION();

		HINSTANCE hInstance = WindowsApplication::GetHInstance();

		if (!s_bRegistered)
		{
			if (!RegisterWindowClass(hInstance, AppClassName))
				return false;

			s_bRegistered = true;
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
		CreateRenderingContext(m_DeviceContext);

		// Raw Input

		RAWINPUTDEVICE mouseRid;
		mouseRid.usUsagePage = 0x01;
		mouseRid.usUsage = 0x02;
		mouseRid.dwFlags = 0;
		mouseRid.hwndTarget = m_WindowHandle;

		if (!RegisterRawInputDevices(&mouseRid, 1, sizeof(mouseRid)))
		{
			LOG_WARN("Cannot register Raw Input devices!");
		}

		return true;
	}

	void WindowsWindow::Show()
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "show the window");
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
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "hide the window");
			return;
		}

		if (m_bVisible)
		{
			m_bVisible = false;

			ShowWindow(m_WindowHandle, SW_HIDE);
		}
	}

	void WindowsWindow::SetTitle(const WString& title)
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "set the title");
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
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "enable the window");
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
			ION_LOG_ENGINE_ERROR(_windowNoInitMessage, "get window dimensions");
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
		if (!m_bFullScreenMode && bFullscreen)
		{
			WINDOWPLACEMENT windowPlacement { };
			windowPlacement.length = sizeof(WINDOWPLACEMENT);

			ionassertnd(GetWindowPlacement(m_WindowHandle, &windowPlacement));

			bool bMaximized = windowPlacement.showCmd == SW_MAXIMIZE;

			MONITORINFO monitorInfo { };
			monitorInfo.cbSize = sizeof(MONITORINFO);

			HMONITOR hMonitor = MonitorFromWindow(m_WindowHandle, MONITOR_DEFAULTTOPRIMARY);
			ionassertnd(GetMonitorInfo(hMonitor, &monitorInfo));

			DWORD style = GetWindowLong(m_WindowHandle, GWL_STYLE);

			m_WindowBeforeFullScreen = SWindowDataBeforeFullScreen {
				windowPlacement,
				style,
				bMaximized,
			};

			// This here changes to "real" fullscreen without any input lag.
			DEVMODE devMode { };
			ionassertnd(EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode));
			ionassertnd(ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);

			ionassertnd(SetWindowLong(m_WindowHandle, GWL_STYLE, (style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP));

			int32 x = monitorInfo.rcMonitor.left;
			int32 y = monitorInfo.rcMonitor.top;
			int32 width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			int32 height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
			ionassertnd(SetWindowPos(m_WindowHandle, HWND_TOP, x, y, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED));

			m_bFullScreenMode = true;

			WindowChangeDisplayModeEvent event((uint64)m_WindowHandle, EDisplayMode::FullScreen);
			PostDeferredEvent(event);
		}
		// Disable
		else if (m_bFullScreenMode && !bFullscreen)
		{
			ionassertnd(SetWindowLong(m_WindowHandle, GWL_STYLE, m_WindowBeforeFullScreen.Style));

			if (m_WindowBeforeFullScreen.bMaximized)
			{
				ionassertnd(ShowWindow(m_WindowHandle, SW_MAXIMIZE));
			}
			else
			{
				RECT& windowRect = m_WindowBeforeFullScreen.WindowPlacement.rcNormalPosition;
				int32 x = windowRect.left;
				int32 y = windowRect.top;
				int32 width = windowRect.right - windowRect.left;
				int32 height = windowRect.bottom - windowRect.top;
				ionassertnd(SetWindowPos(m_WindowHandle, HWND_NOTOPMOST, x, y, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED));
			}

			m_bFullScreenMode = false;

			WindowChangeDisplayModeEvent event((uint64)m_WindowHandle, EDisplayMode::Windowed);
			PostDeferredEvent(event);
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
		RECT rect = { position.x, position.y, position.x, position.y };
		::ClipCursor(&rect);
		m_bCursorLocked = true;
	}

	void WindowsWindow::LockCursor()
	{
		POINT cursorPos { };
		GetCursorPos(&cursorPos);
		RECT rect = { cursorPos.x, cursorPos.y, cursorPos.x, cursorPos.y };
		::ClipCursor(&rect);
		m_bCursorLocked = true;
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

	HGLRC WindowsWindow::CreateRenderingContext(HDC deviceContext, HGLRC parentContext)
	{
		TRACE_FUNCTION();

		if (m_WindowHandle == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "create OpenGL rendering context");
			return NULL;
		}

		// Setup Rendering Context
		m_RenderingContext = OpenGLWindows::CreateGLContext(deviceContext, parentContext);
		return m_RenderingContext;
	}

	void WindowsWindow::DeleteRenderingContext()
	{
		if (m_RenderingContext != NULL)
		{
			wglDeleteContext(m_RenderingContext);
		}
	}

	void WindowsWindow::MakeRenderingContextCurrent()
	{
		// @TODO: Move this whole "MakeContextCurrent" thing into RenderAPI specific files
		// It shouldn't be tied to the window at all

		OpenGLWindows::MakeContextCurrent(m_DeviceContext, m_RenderingContext);
	}

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
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "get the Device Context");
			return NULL;
		}

		return m_DeviceContext;
	}

	HGLRC WindowsWindow::GetRenderingContext() const
	{
		if (m_WindowHandle == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "get the Rendering Context");
			return NULL;
		}

		return m_RenderingContext;
	}

	bool WindowsWindow::s_bRegistered = false;
	bool WindowsWindow::m_bBothShiftsPressed = false;
}
