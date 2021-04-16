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

	WindowsApplication::~WindowsApplication()
	{
		// Shutdown
		Shutdown();
	}

	void WindowsApplication::Start()
	{
#if ION_ENABLE_TRACING
		DebugTracing::Init();
#endif

		// Init
		TRACE_SESSION_BEGIN("Init");
		HINSTANCE hInstance = GetModuleHandle(nullptr);
		InitWindows(hInstance);
		TRACE_SESSION_END();

		// Run
		TRACE_SESSION_BEGIN("Run");
		Run();
		TRACE_SESSION_END();
	}

	void WindowsApplication::InitWindows(HINSTANCE hInstance)
	{
		TRACE_FUNCTION();

		m_HInstance = hInstance;

		LARGE_INTEGER largeInteger { 0 };
		QueryPerformanceFrequency(&largeInteger);
		s_PerformanceFrequency = (float)largeInteger.QuadPart;

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

	void WindowsApplication::DispatchEvent(Event& event)
	{
		TRACE_FUNCTION();

		EventDispatcher dispatcher(event);

		// Handle close event in application
		dispatcher.Dispatch<WindowCloseEvent>(
			[this](WindowCloseEvent& event)
			{
				DestroyWindow((HWND)event.GetWindowHandle());
				return false;
			});

		Application::DispatchEvent(event);
	}

	float WindowsApplication::CalculateFrameTime()
	{
		// @TODO: Would be nice if it were pausable

		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);

		if (s_LastFrameTime == 0.0f)
		{
			s_LastFrameTime = (float)time.QuadPart;
		}

		float difference;
		difference = time.QuadPart - s_LastFrameTime;

		s_LastFrameTime = (float)time.QuadPart;

		return difference / s_PerformanceFrequency;
	}

	HINSTANCE WindowsApplication::m_HInstance;

	float WindowsApplication::s_PerformanceFrequency = 0;
	float WindowsApplication::s_LastFrameTime = 0;

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
