#include "IonPCH.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"

#include "RHI/RHI.h"

#include "UserInterface/ImGui.h"

namespace Ion
{
	WindowsApplication* WindowsApplication::Get()
	{
		return static_cast<WindowsApplication*>(Application::Get());
	}

	void WindowsApplication::Start()
	{
		WindowsApplicationLogger.Info("Starting Windows application.");

		SetThreadDescription(GetCurrentThread(), L"MainThread");
		// Init
		TRACE_SESSION_BEGIN("Init");
		TRACE_RECORD_START();
		HINSTANCE hInstance = GetModuleHandle(nullptr);
		InitWindows(hInstance);
		TRACE_RECORD_STOP();
		TRACE_SESSION_END();

		// Run
		TRACE_SESSION_BEGIN("Run");
		Run();
		TRACE_SESSION_END();
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

	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		TRACE_FUNCTION();

		WindowsApplicationLogger.Info("Initializing Windows application.");

		m_HInstance = hInstance;

		LARGE_INTEGER largeInteger { 0 };
		QueryPerformanceFrequency(&largeInteger);
		s_PerformanceFrequency = (float)largeInteger.QuadPart;

		QueryPerformanceCounter(&s_FirstFrameTime);

		LoadCursors();

		Init();
	}

	WindowsApplication::WindowsApplication(App* clientApp) :
		Application(clientApp),
		m_CurrentCursor(0),
		m_RequestedCursor(0),
		m_CursorHandles()
	{
		WindowsApplicationLogger.Info("Windows application has been created.");
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

		std::static_pointer_cast<WindowsWindow>(GetApplicationWindow())->PollEvents_Application();
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

	void WindowsApplication::OnWindowCloseEvent_Internal(const WindowCloseEvent& event)
	{
		TRACE_FUNCTION();

		WindowsApplicationLogger.Info("Destroying window {{{:#x}}}.", event.WindowHandle);

		DestroyWindow((HWND)event.WindowHandle);
		Application::OnWindowCloseEvent_Internal(event);
	}

	HINSTANCE WindowsApplication::m_HInstance;

	float WindowsApplication::s_PerformanceFrequency = 0;

	// -------------------------------------------------------------
	//  ImGui related  ---------------------------------------------
	// -------------------------------------------------------------

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
