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

#pragma warning(disable:26812)

// ------------------------------
//        Windows Window
// ------------------------------

namespace Ion
{
	static const CStr _windowNoInitMessage = "Cannot {0} before the window is initialized!";

	// Generic Window

	Shared<GenericWindow> GenericWindow::Create()
	{
		return Ion::WindowsWindow::Create();
	}

	// Windows Window

	Shared<WindowsWindow> WindowsWindow::Create()
	{
		ION_LOG_ENGINE_TRACE("Creating Windows window.");
		return Shared<WindowsWindow>(new WindowsWindow);
	}

	WindowsWindow::WindowsWindow() :
		m_WindowHandle(NULL),
		m_DeviceContext(NULL),
		m_RenderingContext(NULL)
	{ }

	void WindowsWindow::PollEvents_Application()
	{
		// Shift hack:
		if (m_bBothShiftsPressed)
		{
			ubyte keyState = 0;
			keyState |= (GetAsyncKeyState(VK_LSHIFT) & 0x8000) >> 14;
			keyState |= (GetAsyncKeyState(VK_RSHIFT) & 0x8000) >> 15;
			if (keyState != 0x3)
			{
				uint actualKeyCode;
				// Left shift held, so right shift released and vice-versa
				if (keyState == 0x2)
					actualKeyCode = Key::RShift;
				else 
					actualKeyCode = Key::LShift;
				m_bBothShiftsPressed = false;

				auto event = KeyReleasedEvent(Key::Shift, actualKeyCode);
				m_EventCallback(event);
			}
		}
	}

	LRESULT WindowsWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		WindowsWindow& windowRef = *(WindowsWindow*)GetProp(hWnd, L"WinObj");

		// --------------------------------------
		// Convert Windows messages to Ion Events
		// --------------------------------------

		// --------------------------
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
			// =============== //
			// Deferred events //
			// =============== //

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

			// =================== //
			// Non-Deferred events //
			// =================== //

			case WM_MOVE:
			{
				POINTS pos = MAKEPOINTS(lParam);
				int xPos = pos.x;
				int yPos = pos.y;
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

			// Keyboard events

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				uint keyCode       = (uint)wParam;
				uint actualKeyCode = keyCode;
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
					ubyte scanCode = LOBYTE(HIWORD(lParam));

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

					auto event = KeyPressedEvent(keyCode, actualKeyCode, bRepeated);
					windowRef.m_EventCallback(event);
				}
				else
				{
					// HACK:
					// Print screen key doesn't send a key down message,
					// so upon release send PressedEvent before the ReleaseEvent.
					// At this point keycode variables are already translated.
					if (keyCode == Key::PrintScr)
					{
						auto printScreenPressed = KeyPressedEvent(keyCode, actualKeyCode, false);
						windowRef.m_EventCallback(printScreenPressed);
					}

					auto event = KeyReleasedEvent(keyCode, actualKeyCode);
					windowRef.m_EventCallback(event);
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
				uint button = MouseButtonFromMessage(uMsg, wParam);;
				bool bState =
					uMsg == WM_LBUTTONDOWN ||
					uMsg == WM_RBUTTONDOWN ||
					uMsg == WM_MBUTTONDOWN ||
					uMsg == WM_XBUTTONDOWN;

				// Down: true, Up: false
				if (bState)
				{
					auto event = MouseButtonPressedEvent(button);
					windowRef.m_EventCallback(event);
				}
				else
				{
					auto event = MouseButtonReleasedEvent(button);
					windowRef.m_EventCallback(event);
				}

				return 0;
			}

			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDBLCLK:
			{
				uint button = MouseButtonFromMessage(uMsg, wParam);

				auto pressedEvent = MouseButtonPressedEvent(button);
				windowRef.m_EventCallback(pressedEvent);

				auto doubleClickEvent = MouseDoubleClickEvent(button);
				windowRef.m_EventCallback(doubleClickEvent);

				return 0;
			}

			case WM_MOUSEWHEEL:
			{
				float delta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
				auto event = MouseScrolledEvent(delta);
				windowRef.m_EventCallback(event);

				return 0;
			}

			case WM_MOUSEMOVE:
			{
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);

				int xPos = GET_X_LPARAM(lParam);
				int yPos = GET_Y_LPARAM(lParam);

				float xNormalised = (float)xPos / clientRect.right;
				float yNormalised = (float)yPos / clientRect.bottom;

				auto event = MouseMovedEvent(xNormalised, yNormalised);
				windowRef.m_EventCallback(event);

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

				uint size;
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
				ubyte* buffer = new ubyte[size];
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
							auto event = RawInputMouseMovedEvent((float)data->lLastX, (float)data->lLastY);
							windowRef.m_EventCallback(event);
						}

						// Mouse Scrolled Event
						if (data->usButtonFlags & RI_MOUSE_WHEEL)
						{
							auto event = RawInputMouseScrolledEvent((float)(short)data->usButtonData);
							windowRef.m_EventCallback(event);
						}

						// Mouse Button Pressed Event
						// 0x0155 - mouse pressed button flags combined
						if (data->usButtonFlags & 0x0155)
						{
							// There can be multiple presses in one message so I have
							// to loop through all of them here so everything gets
							// sent as an event.
							for (uint flag = Bitflag(0); flag != Bitflag(10); flag <<= 2)
							{
								uint buttonFlag = data->usButtonFlags & flag;
								if (buttonFlag)
								{
									uint button;
									switch (buttonFlag)
									{
									case RI_MOUSE_LEFT_BUTTON_DOWN:     button = Mouse::Left;      break;
									case RI_MOUSE_RIGHT_BUTTON_DOWN:    button = Mouse::Right;     break;
									case RI_MOUSE_MIDDLE_BUTTON_DOWN:   button = Mouse::Middle;    break;
									case RI_MOUSE_BUTTON_4_DOWN:        button = Mouse::Button4;   break;
									case RI_MOUSE_BUTTON_5_DOWN:        button = Mouse::Button5;   break;
									}

									auto event = RawInputMouseButtonPressedEvent(button);
									windowRef.m_EventCallback(event);
								}
							}
						}

						// Mouse Button Released Event
						// 0x02AA - mouse released button flags combined
						if (data->usButtonFlags & 0x02AA)
						{
							// Similar loop for button releases
							for (uint flag = Bitflag(1); flag != Bitflag(11); flag <<= 2)
							{
								uint buttonFlag = data->usButtonFlags & flag;
								if (buttonFlag)
								{
									uint button;
									switch (buttonFlag)
									{
									case RI_MOUSE_LEFT_BUTTON_UP:     button = Mouse::Left;      break;
									case RI_MOUSE_RIGHT_BUTTON_UP:    button = Mouse::Right;     break;
									case RI_MOUSE_MIDDLE_BUTTON_UP:   button = Mouse::Middle;    break;
									case RI_MOUSE_BUTTON_4_UP:        button = Mouse::Button4;   break;
									case RI_MOUSE_BUTTON_5_UP:        button = Mouse::Button5;   break;
									}

									auto event = RawInputMouseButtonReleasedEvent(button);
									windowRef.m_EventCallback(event);
								}
							}
						}
					}
				}

				delete[] buffer;
				return 0;
			}

			// =============== //
			// Window messages //
			// =============== //

			case WM_CREATE:
			{
				

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
		WNDCLASS wc = { };

		wc.style = CS_DBLCLKS | CS_OWNDC;
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

		return true;
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

		m_WindowHandle = CreateWindowEx(
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
		if (m_WindowHandle == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "show the window");
			return;
		}

		if (!m_bVisible)
		{
			m_bVisible = true;

			int showWindowCmd = SW_SHOW;
			ShowWindow(m_WindowHandle, showWindowCmd);
		}
	}

	void WindowsWindow::Hide()
	{
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

	void WindowsWindow::SetTitle(const WCStr title)
	{
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
			return { };
		}
		
		RECT windowRect;
		GetWindowRect(m_WindowHandle, &windowRect);

		int width = windowRect.right - windowRect.left;
		int height = windowRect.bottom - windowRect.top;
		return { width, height };
	}

	HGLRC WindowsWindow::CreateRenderingContext(HDC deviceContext)
	{
		if (m_WindowHandle == NULL)
		{
			ION_LOG_ENGINE_CRITICAL(_windowNoInitMessage, "create OpenGL rendering context");
			return NULL;
		}

		// Setup Rendering Context
		m_RenderingContext = OpenGLWindows::CreateGLContext(deviceContext);
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
		OpenGLWindows::MakeContextCurrent(m_DeviceContext, m_RenderingContext);
	}

	void WindowsWindow::SwapBuffers()
	{
		::SwapBuffers(m_DeviceContext);
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

	bool WindowsWindow::m_bBothShiftsPressed = false;
}

