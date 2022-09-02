#include "IonPCH.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "Application/Input/Input.h"

#include "RHI/RHI.h"

#include "UserInterface/ImGui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Ion
{
	Application* const g_pEngineApplication = new WindowsApplication;

	FORCEINLINE static EMouse::Type MouseButtonFromMessage(UINT uMsg, WPARAM wParam)
	{
		if (uMsg <= WM_LBUTTONDBLCLK)
			return EMouse::Left;
		else if (uMsg <= WM_RBUTTONDBLCLK)
			return EMouse::Right;
		else if (uMsg <= WM_MBUTTONDBLCLK)
			return EMouse::Middle;
		else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
			return EMouse::Button4;
		else
			return EMouse::Button5;
	}

	void WindowsApplication::PostEvent(const Event& e)
	{
		g_pEngineApplication->PostEvent(e);
	}

	void WindowsApplication::PostDeferredEvent(const Event& e)
	{
		g_pEngineApplication->PostDeferredEvent(e);
	}

	// Used as a workaround for the double shift release
	static bool bBothShiftsPressed = false;

	LRESULT WindowsApplication::WindowEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		TRACE_FUNCTION();

		TRACE_BEGIN(0, "WindowsWindow - ImGui_ImplWin32_WndProcHandler");
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return 1;
		TRACE_END(0);

		// Read ImGui input first and decide what to do
		// @TODO: This is a quick fix so do something better in the future
		//if (ImGui::GetCurrentContext())
		//{
		//	ImGuiIO& imGuiIO = ImGui::GetIO();
		//	if (imGuiIO.WantCaptureKeyboard)
		//	{
		//		if (IsAnyOf(uMsg,
		//			WM_KEYDOWN,
		//			WM_SYSKEYDOWN,
		//			WM_KEYUP,
		//			WM_KEYUP,
		//			WM_SYSKEYUP))
		//		{
		//			return 0;
		//		}
		//	}
		//	if (imGuiIO.WantCaptureMouse)
		//	{
		//		if (IsAnyOf(uMsg, 
		//			WM_LBUTTONUP,
		//			WM_RBUTTONUP,
		//			WM_MBUTTONUP,
		//			WM_XBUTTONUP,
		//			WM_LBUTTONDOWN,
		//			WM_RBUTTONDOWN,
		//			WM_MBUTTONDOWN,
		//			WM_XBUTTONDOWN,
		//			WM_LBUTTONDBLCLK,
		//			WM_RBUTTONDBLCLK,
		//			WM_MBUTTONDBLCLK,
		//			WM_XBUTTONDBLCLK,
		//			WM_MOUSEWHEEL,
		//			WM_MOUSEMOVE,
		//			WM_INPUT))
		//		{
		//			return 0;
		//		}
		//	}
		//}

		WindowsWindow& window = *(WindowsWindow*)GetProp(hWnd, L"WinObj");

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
				WindowFocusEvent event((void*)hWnd, EVENT_DEBUG_NAME(window.m_Title));
				PostDeferredEvent(event);

				// BUGFIX: When the exclusive fullscreen mode is disrupted in any way
				// (like changing windows) it doesn't continue and there is input lag.
				// @TODO: Might be a weird fix but it works.
				if (window.m_bFullScreenMode)
				{
					window.EnableFullScreen(true);
				}

				return 0;
			}

			case WM_KILLFOCUS:
			{
				WindowLostFocusEvent event((void*)hWnd, EVENT_DEBUG_NAME(window.m_Title));
				PostDeferredEvent(event);

				return 0;
			}

			case WM_CLOSE:
			{
				WindowCloseEvent event((void*)hWnd, EVENT_DEBUG_NAME(window.m_Title));
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
				WindowMovedEvent event((void*)hWnd, xPos, yPos, EVENT_DEBUG_NAME(window.m_Title));
				PostEvent(event);

				return 0;
			}

			case WM_SIZE:
			{
				int32 width  = LOWORD(lParam);
				int32 height = HIWORD(lParam);
				WindowResizeEvent event((void*)hWnd, width, height, EVENT_DEBUG_NAME(window.m_Title));
				PostEvent(event);

				return 0;
			}

			case WM_PAINT:
			{
				// @TODO: Make Paint event when in modal loop

				break;
			}

			case WM_SYSCHAR:
			{
				break;
			}

			case WM_SETCURSOR:
			{
				if (LOWORD(lParam) == HTCLIENT)
				{
					if (WindowsApplication::Get()->UpdateMouseCursor())
						return 1;
				}
				break;
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
						bBothShiftsPressed = true;
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
				InputManager::TranslateNativeKeyCode(&keyCode);
				InputManager::TranslateNativeKeyCode(&actualKeyCode);

				// HACK:
				// Windows doesn't have a separate keycode for keypad Enter button,
				// but the key is extended in this case. Therefore, if the keycode
				// is an extended Enter key, change the internal (translated)
				// keycode to KP_Enter.
				if (keyCode == EKey::Enter && bExtendedKey)
				{
					keyCode       = EKey::KP_Enter;
					actualKeyCode = EKey::KP_Enter;
				}

				// Down: true, Up: false
				if (bState)
				{
					// If the key is already pressed it means it was repeated
					bool bRepeated = InputManager::IsKeyPressed((EKey::Type)actualKeyCode);

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
					if (keyCode == EKey::PrintScr)
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
				RECT clientRect { };
				GetClientRect(hWnd, &clientRect);

				int32 xPos = GET_X_LPARAM(lParam);
				int32 yPos = GET_Y_LPARAM(lParam);

				float xNormalised = (float)xPos / clientRect.right;
				float yNormalised = (float)yPos / clientRect.bottom;

				int32 ssXPos = xPos + clientRect.left;
				int32 ssYPos = yPos + clientRect.top;

				MouseMovedEvent event((float)xNormalised, (float)yNormalised, ssXPos, ssYPos);
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
				if (!InputManager::IsRawInputAvailable())
					break;

				uint32 size;
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
				uint8* buffer = (uint8*)_alloca(size);
				if (!buffer)
					return 0;

				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size)
				{
					WindowsApplicationLogger.Warn("GetRawInputData does not return correct size!");
					return 0;
				}

				RAWINPUT* rawInput = (RAWINPUT*)buffer;

				if (InputManager::IsRawInputEnabled())
				{
					// Mouse Input
					if (rawInput->header.dwType == RIM_TYPEMOUSE)
					{
						RAWMOUSE* data = &rawInput->data.mouse;

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
									case RI_MOUSE_LEFT_BUTTON_DOWN:     button = EMouse::Left;      break;
									case RI_MOUSE_RIGHT_BUTTON_DOWN:    button = EMouse::Right;     break;
									case RI_MOUSE_MIDDLE_BUTTON_DOWN:   button = EMouse::Middle;    break;
									case RI_MOUSE_BUTTON_4_DOWN:        button = EMouse::Button4;   break;
									case RI_MOUSE_BUTTON_5_DOWN:        button = EMouse::Button5;   break;
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
									case RI_MOUSE_LEFT_BUTTON_UP:     button = EMouse::Left;      break;
									case RI_MOUSE_RIGHT_BUTTON_UP:    button = EMouse::Right;     break;
									case RI_MOUSE_MIDDLE_BUTTON_UP:   button = EMouse::Middle;    break;
									case RI_MOUSE_BUTTON_4_UP:        button = EMouse::Button4;   break;
									case RI_MOUSE_BUTTON_5_UP:        button = EMouse::Button5;   break;
									}

									RawInputMouseButtonReleasedEvent event(button);
									PostEvent(event);
								}
							}
						}
					}
				}
				return 0;
			}
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	WindowsApplication::WindowsApplication() :
		m_CurrentCursor(0),
		m_RequestedCursor(0),
		m_CursorHandles()
	{
		WindowsApplicationLogger.Info("Windows application has been created.");
	}

	WindowsApplication* WindowsApplication::Get()
	{
		return static_cast<WindowsApplication*>(g_pEngineApplication);
	}

	static LARGE_INTEGER s_FirstFrameTime;
	static float s_LastFrameTime = 0;

	void WindowsApplication::SetCursor(ECursorType cursor)
	{
		m_RequestedCursor = (int32)cursor;
	}

	ECursorType WindowsApplication::GetCurrentCursor() const
	{
		// The current cursor will not get updated until the next frame,
		// so if it's going to change, return the requested cursor instead.
		if (m_RequestedCursor != -1)
			return (ECursorType)m_RequestedCursor;
		return (ECursorType)m_CurrentCursor;
	}

	void WindowsApplication::PlatformInit()
	{
		TRACE_FUNCTION();

		WindowsApplicationLogger.Info("Initializing Windows application.");

		SetThreadDescription(GetCurrentThread(), L"MainThread");

		m_HInstance = GetModuleHandle(nullptr);

		LARGE_INTEGER largeInteger { 0 };
		QueryPerformanceFrequency(&largeInteger);
		s_PerformanceFrequency = (float)largeInteger.QuadPart;

		QueryPerformanceCounter(&s_FirstFrameTime);

		LoadCursors();
	}

	void WindowsApplication::PlatformShutdown()
	{
		TRACE_FUNCTION();

		WindowsApplicationLogger.Info("Shutting down Windows application.");
	}

	void WindowsApplication::PollEvents()
	{
		TRACE_FUNCTION();

		MSG message;

		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		EventFixes();
	}

	void WindowsApplication::Update(float DeltaTime)
	{
		TRACE_FUNCTION();

		if (m_CurrentCursor != m_RequestedCursor)
			UpdateMouseCursor();

		SetCursor(ECursorType::NoChange);

		Application::Update(DeltaTime);
	}

	void WindowsApplication::Render()
	{
		TRACE_FUNCTION();

		Application::Render();
	}

	float Application::CalculateFrameTime()
	{
		// @TODO: Would be nice if it were pausable

		LARGE_INTEGER liTime;
		QueryPerformanceCounter(&liTime);

		float time = (float)(liTime.QuadPart - s_FirstFrameTime.QuadPart);

		if (s_LastFrameTime == 0.0f)
		{
			s_LastFrameTime = time;
			// Return 1/60s for the first time
			return 1.0f / 60.0f;
		}

		float difference;
		difference = time - s_LastFrameTime;

		s_LastFrameTime = time;

		return difference / WindowsApplication::s_PerformanceFrequency;
	}

	void WindowsApplication::OnWindowCloseEvent(const WindowCloseEvent& e)
	{
		TRACE_FUNCTION();

		WindowsApplicationLogger.Info("Destroying window {{{}}}.", e.WindowHandle);

		DestroyWindow((HWND)e.WindowHandle);

		Application::OnWindowCloseEvent(e);
	}

	HINSTANCE WindowsApplication::m_HInstance;

	float WindowsApplication::s_PerformanceFrequency = 0;

	void WindowsApplication::LoadCursors()
	{
		WindowsApplicationLogger.Info("Loading cursors.");

		constexpr size_t count = (size_t)ECursorType::_Count;
		String cursorPath;
		for (int32 i = 0; i < count; ++i)
		{
			HCURSOR& handle = m_CursorHandles[i];
			ECursorType currentCursor = (ECursorType)i;
			switch (currentCursor)
			{
			case ECursorType::Arrow:
				handle = LoadCursor(NULL, IDC_ARROW);
				break;
			case ECursorType::Help:
				handle = LoadCursor(NULL, IDC_HELP);
				break;
			case ECursorType::Cross:
				handle = LoadCursor(NULL, IDC_CROSS);
				break;
			case ECursorType::TextEdit:
				handle = LoadCursor(NULL, IDC_IBEAM);
				break;
			case ECursorType::Unavailable:
				handle = LoadCursor(NULL, IDC_NO);
				break;
			case ECursorType::UpArrow:
				handle = LoadCursor(NULL, IDC_UPARROW);
				break;
			case ECursorType::ResizeNS:
				handle = LoadCursor(NULL, IDC_SIZENS);
				break;
			case ECursorType::ResizeWE:
				handle = LoadCursor(NULL, IDC_SIZEWE);
				break;
			case ECursorType::ResizeNWSE:
				handle = LoadCursor(NULL, IDC_SIZENWSE);
				break;
			case ECursorType::ResizeNESW:
				handle = LoadCursor(NULL, IDC_SIZENESW);
				break;
			case ECursorType::Move:
				handle = LoadCursor(NULL, IDC_SIZEALL);
				break;
			case ECursorType::Hand:
				handle = LoadCursor(NULL, IDC_HAND);
				break;
			case ECursorType::Grab:
				// @TODO: unhardcode these paths
				cursorPath = (EnginePath::GetEngineContentPath() + "Cursor/openhand.cur").ToString();
				handle = LoadCursorFromFile(StringConverter::StringToWString(cursorPath).c_str());
				if (!handle)
				{
					WindowsApplicationLogger.Warn("Could not load cursor {0}. The default one will be used.", cursorPath);
					handle = LoadCursor(NULL, IDC_ARROW);
				}
				break;
			case ECursorType::GrabClosed:
				cursorPath = (EnginePath::GetEngineContentPath() + "Cursor/closedhand.cur").ToString();
				handle = LoadCursorFromFile(StringConverter::StringToWString(cursorPath).c_str());
				if (!handle)
				{
					WindowsApplicationLogger.Warn("Could not load cursor {0}. The default one will be used.", cursorPath);
					handle = LoadCursor(NULL, IDC_ARROW);
				}
				break;
			default:
				ionassert(0, "Cursor type not implemented.");
				handle = LoadCursor(NULL, IDC_ARROW);
			}
		}
		m_CurrentCursor = 0;
	}

	bool WindowsApplication::UpdateMouseCursor()
	{
		if (m_RequestedCursor == (int32)ECursorType::NoChange) // Don't change the cursor
			return false;

		m_CurrentCursor = m_RequestedCursor;

		HCURSOR handle = m_CursorHandles[m_CurrentCursor];
		::SetCursor(handle);
		return true;
	}

	void WindowsApplication::EventFixes()
	{
		// Shift hack:
		if (bBothShiftsPressed)
		{
			uint8 keyState = 0;
			keyState |= (GetAsyncKeyState(VK_LSHIFT) & 0x8000) >> 14;
			keyState |= (GetAsyncKeyState(VK_RSHIFT) & 0x8000) >> 15;
			if (keyState != 0x3)
			{
				uint32 actualKeyCode;
				// Left shift held, so right shift released and vice-versa
				if (keyState == 0x2)
					actualKeyCode = EKey::RShift;
				else
					actualKeyCode = EKey::LShift;
				bBothShiftsPressed = false;

				KeyReleasedEvent event(EKey::Shift, actualKeyCode);
				PostEvent(event);
			}
		}
	}

	// -------------------------------------------------------------
	//  ImGui related  ---------------------------------------------
	// -------------------------------------------------------------

	void WindowsApplication::InitImGuiBackend(const std::shared_ptr<GenericWindow>& window) const
	{
		TRACE_FUNCTION();

		WindowsApplicationLogger.Info("Initializing ImGui Windows backend.");

		std::shared_ptr<WindowsWindow> windowsWindow = std::static_pointer_cast<WindowsWindow>(window);
		ImGui_ImplWin32_Init((HWND)windowsWindow->GetNativeHandle());
		RHI::Get()->InitImGuiBackend();
	}

	void WindowsApplication::ImGuiNewFramePlatform() const
	{
		TRACE_FUNCTION();

		RHI::Get()->ImGuiNewFrame();
		ImGui_ImplWin32_NewFrame();
	}

	void WindowsApplication::ImGuiRenderPlatform(ImDrawData* drawData) const
	{
		TRACE_FUNCTION();

		std::shared_ptr<WindowsWindow> window = std::static_pointer_cast<WindowsWindow>(GetWindow());

		RHI::Get()->ImGuiRender(drawData);

		// @TODO: If viewports enabled:

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void WindowsApplication::ImGuiShutdownPlatform() const
	{
		TRACE_FUNCTION();

		WindowsApplicationLogger.Info("Shutting down ImGui Windows backend.");

		RHI::Get()->ImGuiShutdown();
		ImGui_ImplWin32_Shutdown();
	}
}
