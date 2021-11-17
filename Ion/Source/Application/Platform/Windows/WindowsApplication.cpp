#include "IonPCH.h"

#include "WindowsApplication.h"
#include "WindowsWindow.h"
#include "Core/Platform/Windows/WindowsInput.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/Windows/OpenGLWindows.h"

#include "UserInterface/ImGui.h"

namespace Ion
{
	WindowsApplication* WindowsApplication::Get()
	{
		return static_cast<WindowsApplication*>(Application::Get());
	}

	void WindowsApplication::Start()
	{
#if ION_ENABLE_TRACING
		DebugTracing::Init();
#endif

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

	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		TRACE_FUNCTION();

		m_HInstance = hInstance;

		LARGE_INTEGER largeInteger { 0 };
		QueryPerformanceFrequency(&largeInteger);
		s_PerformanceFrequency = (float)largeInteger.QuadPart;

		QueryPerformanceCounter(&s_FirstFrameTime);

		Init();
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
		}

		float difference;
		difference = time - s_LastFrameTime;

		s_LastFrameTime = time;

		return difference / WindowsApplication::s_PerformanceFrequency;
	}

	void WindowsApplication::OnWindowCloseEvent_Internal(const WindowCloseEvent& event)
	{
		DestroyWindow((HWND)event.GetWindowHandle());
		Application::OnWindowCloseEvent_Internal(event);
	}

	HINSTANCE WindowsApplication::m_HInstance;

	float WindowsApplication::s_PerformanceFrequency = 0;

	// -------------------------------------------------------------
	//  ImGui related  ---------------------------------------------
	// -------------------------------------------------------------

	void WindowsApplication::InitImGuiBackend(const TShared<GenericWindow>& window) const
	{
		TRACE_FUNCTION();

		TShared<WindowsWindow> windowsWindow = std::static_pointer_cast<WindowsWindow>(window);
		ImGui_ImplWin32_Init((HWND)windowsWindow->GetNativeHandle());
		RenderAPI::InitImGuiBackend();
	}

	void WindowsApplication::ImGuiNewFramePlatform() const
	{
		TRACE_FUNCTION();

		RenderAPI::ImGuiNewFrame();
		ImGui_ImplWin32_NewFrame();
	}

	void WindowsApplication::ImGuiRenderPlatform(ImDrawData* drawData) const
	{
		TRACE_FUNCTION();

		TShared<WindowsWindow> window = std::static_pointer_cast<WindowsWindow>(GetWindow());

		RenderAPI::ImGuiRender(drawData);

		// @TODO: If viewports enabled:

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void WindowsApplication::ImGuiShutdownPlatform() const
	{
		TRACE_FUNCTION();

		RenderAPI::ImGuiShutdown();
		ImGui_ImplWin32_Shutdown();
	}
}
