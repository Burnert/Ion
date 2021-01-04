#include "IonPCH.h"

#include "WindowsWindow.h"
#include "WindowsApplication.h"

#include "Core/Event/Event.h"
#include "Core/Event/WindowEvent.h"
#include "Core/Event/InputEvent.h"
#include "Core/Input/Input.h"
#include "Core/Platform/Windows/WindowsInput.h"

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
	{ }

	void WindowsWindow::Update_Application()
	{
		// Shift hack:
		if (m_bBothShiftsPressed)
		{
			byte keyState = 0;
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
			//                 //
			// Deferred events //
			//                 //

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

			//                     //
			// Non-Deferred events //
			//                     //

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
					// Rest of the code in Update_Application()
					if ((GetKeyState(VK_LSHIFT) & 0x8000) &&
						(GetKeyState(VK_RSHIFT) & 0x8000))
					{
						m_bBothShiftsPressed = true;
					}

					// Note:
					// Distinguishing between left and right Shift keys
					// is a bit diffrent from the other keys.
					byte scanCode = LOBYTE(HIWORD(lParam));

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
					bool bRepeated = WindowsApplication::Get()->GetInputManager()->IsKeyPressed((KeyCode)actualKeyCode);

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
				uint button;
				bool bState = 
					uMsg == WM_LBUTTONDOWN ||
					uMsg == WM_RBUTTONDOWN ||
					uMsg == WM_MBUTTONDOWN ||
					uMsg == WM_XBUTTONDOWN;

				if (uMsg <= WM_LBUTTONDBLCLK)
					button = Mouse::Left;
				else if (uMsg <= WM_RBUTTONDBLCLK)
					button = Mouse::Right;
				else if (uMsg <= WM_MBUTTONDBLCLK)
					button = Mouse::Middle;
				else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
					button = Mouse::Button4;
				else
					button = Mouse::Button5;
					
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

			case WM_MOUSEWHEEL:
			{
				float delta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
				auto event = MouseScrolledEvent(delta);
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

	bool WindowsWindow::m_bBothShiftsPressed = false;
}

