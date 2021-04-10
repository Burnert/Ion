#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Input/Input.h"

#include "glad/glad.h"

#include "RenderAPI/RenderAPI.h"
#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

namespace Ion
{
	Application* Application::Get()
	{
		// This goes off when Application was not yet created.
		// (shouldn't ever happen, unless you're doing something weird)
		ASSERT(s_Instance);
		return s_Instance;
	}

	Application::Application() :
		m_EventQueue(MakeUnique<EventQueue>()),
		m_LayerStack(MakeUnique<LayerStack>()),
		m_MainThreadId(std::this_thread::get_id()),
		m_bRunning(true)
	{
		m_EventQueue->SetEventHandler(BIND_METHOD_1P(Application::DispatchEvent));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		m_InputManager = InputManager::Create();

		RenderAPI::Init(ERenderAPI::OpenGL);
		COUNTER_TIME_DATA(timeRenderApiInit, "RenderAPI_InitTime");
		LOG_INFO("{0}: {1}s", timeRenderApiInit.Name, ((float)timeRenderApiInit.GetTimeMs()) * 0.001f);

		// Create a platform specific window.
		m_Window = GenericWindow::Create();

		// Current thread will render graphics in this window.
		m_Window->Initialize();

		m_Window->SetEventCallback(BIND_METHOD_1P(Application::PostEvent));
		m_Window->SetDeferredEventCallback(BIND_METHOD_1P(Application::PostDeferredEvent));

		m_Renderer = Renderer::Create();
		m_Renderer->Init();

		m_Renderer->SetVSyncEnabled(false);

		InitImGui();

		const char* renderAPILabel = RenderAPI::GetCurrentDisplayName();

		std::wstring windowTitle = TEXT("Ion - ");
		wchar renderAPILabelW[120];
		// @TODO: This is from Windows, change that:
		MultiByteToWideChar(CP_UTF8, 0, renderAPILabel, -1, renderAPILabelW, 120);
		windowTitle += renderAPILabelW;
		m_Window->SetTitle(windowTitle);

		m_Window->Show();

		// Call client overriden Init function
		OnInit();

		Run();

		ImGuiShutdownPlatform();
		ImGui::DestroyContext();

		// Call client overriden Shutdown function
		OnShutdown();
	}

	void Application::PollEvents()
	{
		// Platform specific
	}

	void Application::Update(float deltaTime)
	{
		ImGuiNewFramePlatform();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		m_LayerStack->OnUpdate(deltaTime);
		OnUpdate(deltaTime);
	}

	void Application::Render()
	{
		ImGui::Render();

		m_LayerStack->OnRender();
		OnRender();

		ImGuiRenderPlatform(ImGui::GetDrawData());

		m_Window->SwapBuffers();
	}

	void Application::PostEvent(Event& event)
	{
		DispatchEvent(event);
	}

	void Application::PostDeferredEvent(DeferredEvent& event)
	{
		m_EventQueue->PushEvent(std::move(event));
	}

	void Application::DispatchEvent(Event& event)
	{
		//ION_LOG_ENGINE_DEBUG("Event: {0}", event.Debug_ToString());

		EventDispatcher dispatcher(event);

		// Handle close event in application
		dispatcher.Dispatch<WindowCloseEvent>(
			[this](WindowCloseEvent& event)
			{
				m_bRunning = false;
				return false;
			});

		dispatcher.Dispatch<WindowResizeEvent>(
			[this](WindowResizeEvent& event)
			{
				int width = (int)event.GetWidth();
				int height = (int)event.GetHeight();

				m_Renderer->SetViewportDimensions(SViewportDimensions { 0, 0, width, height });

				return true;
			});

		dispatcher.Dispatch<WindowChangeDisplayModeEvent>(
			[this](WindowChangeDisplayModeEvent& event)
			{
				GetWindow()->ClipCursor();
				//GetWindow()->ShowCursor(false);
				return false;
			});

		dispatcher.Dispatch<KeyPressedEvent>(
			[this](KeyPressedEvent& event)
			{
				// Toggle wireframe display with 1 key
				if (event.GetKeyCode() == Key::One)
				{
					EPolygonDrawMode drawMode = (GetRenderer()->GetPolygonDrawMode() == EPolygonDrawMode::Fill) ?
						EPolygonDrawMode::Lines : EPolygonDrawMode::Fill;
					GetRenderer()->SetPolygonDrawMode(drawMode);
				}
				// Toggle VSync with 2 key
				else if (event.GetKeyCode() == Key::Two)
				{
					bool vsync = GetRenderer()->GetVSyncEnabled();
					GetRenderer()->SetVSyncEnabled(!vsync);
				}
				// Toggle fullscreen with Alt + Enter
				else if (event.GetKeyCode() == Key::Enter)
				{
					if (GetInputManager()->IsKeyPressed(Key::LAlt))
					{
						bool bFullScreen = GetWindow()->IsFullScreenEnabled();
						GetWindow()->EnableFullScreen(!bFullScreen);
					}
				}
				else if (event.GetKeyCode() == Key::F1)
				{
					if (GetInputManager()->IsKeyPressed(Key::LShift))
					{
						GetWindow()->UnlockCursor();
						GetWindow()->ShowCursor(true);
					}
				}
				return true;
			});

		// Lock cursor on client area click
		dispatcher.Dispatch<MouseButtonPressedEvent>(
			[this](MouseButtonPressedEvent& event)
			{
				// Lock when clicking on client area
				if (!GetWindow()->IsCursorLocked())
				{
					GetWindow()->ClipCursor();
					//GetWindow()->ShowCursor(false);
				}
				return false;
			});

		dispatcher.Dispatch<WindowFocusEvent>(
			[this](WindowFocusEvent& event)
			{
				// Lock the cursor only if a fullscreen window has been focused
				// If we lock on a non-fullscreen one, it'll hide the cursor immediately
				// instead of doing it after clicking the client area
				if (GetWindow()->IsFullScreenEnabled())
				{
					GetWindow()->ClipCursor();
					//GetWindow()->ShowCursor(false);
				}
				return false;
			});

		// @TODO: Input is still captured even when clicking on a non-client area.

		dispatcher.Dispatch<WindowLostFocusEvent>(
			[this](WindowLostFocusEvent& event)
			{
				GetWindow()->UnlockCursor();
				GetWindow()->ShowCursor(true);
				return false;
			});

		m_InputManager->OnEvent(event);
		m_LayerStack->OnEvent(event);
		OnEvent(event);
	}

	void Application::Run()
	{
		// Application loop
		while (m_bRunning)
		{
			PollEvents();

			float deltaTime = CalculateFrameTime();
			Update(deltaTime);
			Render();

			m_EventQueue->ProcessEvents();
		}
	}

	float Application::CalculateFrameTime()
	{
		LOG_WARN("Cannot calculate frame time on this platform!");
		return 0.0f;
	}

	void Application::InitImGui() const
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& imGuiIO = ImGui::GetIO();
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#if defined PLATFORM_SUPPORTS_DIRECTX
		if (RenderAPI::GetCurrent() == ERenderAPI::DirectX)
#elif defined PLATFORM_SUPPORTS_OPENGL && PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL
		if (RenderAPI::GetCurrent() == ERenderAPI::OpenGL)
#endif
		{
			imGuiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& imGuiStyle = ImGui::GetStyle();
		imGuiStyle.WindowRounding = 0.0f;
		imGuiStyle.Colors[ImGuiCol_WindowBg].w = 1.0f;

		// Setup Platform/Renderer backends
		InitImGuiBackend(m_Window);
	}

	Application* Application::s_Instance = nullptr;
}
