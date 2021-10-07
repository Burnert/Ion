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
		ionassert(s_Instance);
		return s_Instance;
	}

	Application::Application() :
		m_EventQueue(MakeUnique<EventQueue<EventHandler>>()),
		m_LayerStack(MakeUnique<LayerStack>()),
		m_MainThreadId(std::this_thread::get_id()),
		m_bRunning(true)
	{
		Logger::Init();
	}

	void Application::Start()
	{
		ionassert(0, "Start function not implemented!");
	}

	void Application::Init()
	{
		TRACE_FUNCTION();

		m_InputManager = InputManager::Create();

		RenderAPI::Init(ERenderAPI::OpenGL);

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

		m_BaseWindowTitle = TEXT("Ion - ");
		m_BaseWindowTitle += StringConverter::StringToWString(renderAPILabel);
		m_Window->SetTitle(m_BaseWindowTitle);

		m_Window->Show();

		TRACE_BEGIN(0, "Application - Client::OnInit");
		// Call client overriden Init function
		OnInit();
		TRACE_END(0);
	}

	void Application::Shutdown()
	{
		TRACE_FUNCTION();

		ShutdownImGui();

		TRACE_BEGIN(0, "Application - Client::OnShutdown");
		// Call client overriden Shutdown function
		OnShutdown();
		TRACE_END(0);
	}

	void Application::PollEvents()
	{
		// Platform specific
	}

	void Application::Update(float deltaTime)
	{
		TRACE_FUNCTION();

		ImGuiNewFramePlatform();
		ImGui::NewFrame();

		wchar fps[30];
		swprintf_s(fps, L" (%.2f FPS)", 1.0f / deltaTime);
		m_Window->SetTitle(m_BaseWindowTitle + fps);

		m_LayerStack->OnUpdate(deltaTime);

		TRACE_BEGIN(0, "Application - Client::OnUpdate");
		OnUpdate(deltaTime);
		TRACE_END(0);
	}

	void Application::Render()
	{
		TRACE_FUNCTION();

		m_LayerStack->OnRender();

		TRACE_BEGIN(0, "Application - Client::OnRender");
		OnRender();
		TRACE_END(0);

		ImGui::Render();
		ImGuiRenderPlatform(ImGui::GetDrawData());

		m_Window->SwapBuffers();
	}

	void Application::PostEvent(Event& event)
	{
		DispatchEvent(event);
	}

	void Application::PostDeferredEvent(Event& event)
	{
		m_EventQueue->PushEvent(event);
	}

	void Application::DispatchEvent(const Event& event)
	{
		TRACE_FUNCTION();

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
				int32 width = (int32)event.GetWidth();
				int32 height = (int32)event.GetHeight();

				m_Renderer->SetViewportDimensions(SViewportDimensions { 0, 0, width, height });

				return true;
			});

		dispatcher.Dispatch<KeyPressedEvent>(
			[this](KeyPressedEvent& event)
			{
				// Toggle VSync with 2 key
				if (event.GetKeyCode() == Key::Two)
				{
					bool vsync = GetRenderer()->IsVSyncEnabled();
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

		m_InputManager->OnEvent(event);
		m_LayerStack->OnEvent(event);

		TRACE_BEGIN(0, "Application - Client::OnEvent");
		OnEvent(event);
		TRACE_END(0);
	}

	void Application::Run()
	{
		TRACE_FUNCTION();

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

	void Application::InitImGui() const
	{
		TRACE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& imGuiIO = ImGui::GetIO();
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		if (RenderAPI::GetCurrent() == ERenderAPI::DirectX
#if defined PLATFORM_SUPPORTS_OPENGL && PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL
			|| RenderAPI::GetCurrent() == ERenderAPI::OpenGL
#endif
			)
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

	void Application::ShutdownImGui() const
	{
		TRACE_FUNCTION();

		ImGuiShutdownPlatform();
		ImGui::DestroyContext();
	}

	Application* Application::s_Instance = nullptr;

	wchar* Application::s_EnginePath = nullptr;
}
