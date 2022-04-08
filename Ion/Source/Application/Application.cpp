#include "IonPCH.h"

#include "Application.h"
#include "GameLayer.h"
#include "ImGuiLayer.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Input/Input.h"

#include "Engine/Engine.h"

#include "RenderAPI/RenderAPI.h"
#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

#include "IonApp.h"

namespace Ion
{
	void Application::SetCursor(ECursorType cursor)
	{
		LOG_WARN("Changing the cursor is not supported on this platform.");
	}

	ECursorType Application::GetCurrentCursor() const
	{
		return ECursorType::Arrow;
	}

	Application* Application::Get()
	{
		// This goes off when Application was not yet created.
		// (shouldn't ever happen, unless you're doing something weird)
		ionassert(s_Instance);
		return s_Instance;
	}

	Renderer* Application::GetRenderer()
	{
		return Renderer::Get();
	}

	Application::Application(App* clientApp) :
		m_EventDispatcher(this),
		m_EventQueue(MakeUnique<EventQueue<EventHandler>>()),
		m_LayerStack(MakeUnique<LayerStack>()),
		m_MainThreadId(std::this_thread::get_id()),
		m_bRunning(true),
		m_ClientApp(clientApp)
	{
		ionassert(clientApp);
		Logger::Init();
	}

	Application::~Application()
	{
		Shutdown();
	}

	void Application::Start()
	{
		ionassert(0, "Start function not implemented!");
	}

	void Application::Init()
	{
		TRACE_FUNCTION();

		m_InputManager = InputManager::Create();

		// Create a platform specific window.
		m_Window = GenericWindow::Create();
		m_Window->Initialize();

		g_Engine->Init();

		// Current thread will render graphics in this window.
		RenderAPI::Init(ERenderAPI::DX11, m_Window.get());

		Renderer* renderer = Renderer::Create();
		renderer->Init();
		renderer->SetVSyncEnabled(false);

		AssetManager::Init();

		InitImGui();
		LoadFonts();

		// @TODO: This should be in the Runtime eventually
		//m_LayerStack->PushLayer<GameLayer>("GameLayer");
		//m_LayerStack->PushOverlayLayer<ImGuiLayer>("ImGui");

		SetApplicationTitle(L"Ion");
		SetupWindowTitle();
		m_Window->Show();

		TRACE_BEGIN(0, "Application - Client::OnInit");
		// Call client overriden Init function
		OnInit();
		m_ClientApp->OnInit();
		TRACE_END(0);
	}

	void Application::Shutdown()
	{
		TRACE_FUNCTION();

		ShutdownImGui();

		TRACE_BEGIN(0, "Application - Client::OnShutdown");
		// Call client overriden Shutdown function
		OnShutdown();
		m_ClientApp->OnShutdown();
		TRACE_END(0);

		AssetManager::Shutdown();

		RenderAPI::Shutdown();

		g_Engine->Shutdown();
	}

	void Application::Exit()
	{
		// @TODO: Exit
		LOG_CRITICAL("TODO: Implement Exit!");
	}

	void Application::PollEvents()
	{
		// Platform specific
	}

	void Application::Update(float deltaTime)
	{
		TRACE_FUNCTION();

		// Don't reset the cursor if the mouse is being held
		if (!InputManager::IsMouseButtonPressed(Mouse::Left))
		{
			// Reset the cursor
			SetCursor(ECursorType::Arrow);
		}

		g_Engine->Update(deltaTime);

		ImGuiNewFramePlatform();
		ImGui::NewFrame();

		// @TODO: This is broken and kills CPU
		//UpdateWindowTitle(deltaTime);

		AssetManager::Update();
		
		TRACE_BEGIN(0, "Application - Client::OnUpdate");
		OnUpdate(deltaTime);
		m_ClientApp->OnUpdate(deltaTime);
		TRACE_END(0);

		m_LayerStack->OnUpdate(deltaTime);

		// Arrow means that the cursor was not overriden by Ion,
		// so ImGui can override it.
		ECursorType cursor = GetCurrentCursor();
		if (cursor == ECursorType::Arrow)
		{
			ImGuiMouseCursor imGuiCursor = ImGui::GetMouseCursor();
			SetImGuiCursor(imGuiCursor);
		}
	}

	void Application::Render()
	{
		TRACE_FUNCTION();

		ImGui::Render();

		RenderAPI::BeginFrame();

		TRACE_BEGIN(0, "Application - Client::OnRender");
		OnRender();
		m_ClientApp->OnRender();
		TRACE_END(0);

		m_LayerStack->OnRender();

		// Render to the window framebuffer last
		Renderer::Get()->SetRenderTarget(nullptr);

		{
			TRACE_SCOPE("Render ImGui");
			ImGuiRenderPlatform(ImGui::GetDrawData());
		}

		RenderAPI::EndFrame(*m_Window);
	}

	void Application::CallClientAppOnEvent(const Event& event)
	{
		m_ClientApp->OnEvent(event);
	}

	void Application::SetApplicationTitle(const WString& title)
	{
		m_ApplicationTitle = title;
		SetupWindowTitle();
	}

	void Application::SetupWindowTitle()
	{
		WString title = m_ApplicationTitle;
		title += L" - ";
		title += StringConverter::StringToWString(RenderAPI::GetCurrentDisplayName());
		m_Window->SetTitle(title);
	}

	void Application::UpdateWindowTitle(float deltaTime)
	{
		//wchar fps[30];
		//swprintf_s(fps, L" (%.2f FPS)", 1.0f / deltaTime);
		//m_Window->SetTitle(m_BaseWindowTitle + fps);
	}

	/*
	void Application::DispatchEvent(const Event& event)
	{
		TRACE_FUNCTION();

		dispatcher.Dispatch<KeyPressedEvent>(
			[this](KeyPressedEvent& event)
			{
				// Toggle VSync with 2 key
				if (event.GetKeyCode() == Key::Two)
				{
					bool vsync = GetRenderer()->IsVSyncEnabled();
					GetRenderer()->SetVSyncEnabled(!vsync);
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
	}
	*/

	void Application::OnWindowCloseEvent_Internal(const WindowCloseEvent& event)
	{
		m_bRunning = false;
	}

	void Application::OnWindowResizeEvent_Internal(const WindowResizeEvent& event)
	{
		int32 width = (int32)event.GetWidth();
		int32 height = (int32)event.GetHeight();

		// Cannot create a texture with a width or height of 0.
		// This happens when the window is minimized
		if (width == 0 || height == 0)
			return;

		Renderer::Get()->SetViewportDimensions(ViewportDimensions { 0, 0, width, height });
	}

	void Application::OnWindowChangeDisplayModeEvent_Internal(const WindowChangeDisplayModeEvent& event)
	{
		ViewportDimensions dimensions { };
		dimensions.Width = event.GetWidth();
		dimensions.Height = event.GetHeight();
	}

	void Application::OnKeyPressedEvent_Internal(const KeyPressedEvent& event)
	{
		// Toggle fullscreen with Alt + Enter
		if (event.GetKeyCode() == Key::Enter)
		{
			if (GetInputManager()->IsKeyPressed(Key::LAlt))
			{
				LOG_DEBUG("Fullscreen Toggle");

				bool bFullScreen = GetWindow()->IsFullScreenEnabled();
				GetWindow()->EnableFullScreen(!bFullScreen);
			}
		}
		// Exit application with Alt + F4
		else if (event.GetKeyCode() == Key::F4)
		{
			if (GetInputManager()->IsKeyPressed(Key::LAlt))
			{
				Exit();
			}
		}
	}

	void Application::OnKeyReleasedEvent_Internal(const KeyReleasedEvent& event)
	{

	}

	void Application::OnKeyRepeatedEvent_Internal(const KeyRepeatedEvent& event)
	{

	}

	void Application::Run()
	{
		TRACE_FUNCTION();

		// Application loop
		while (m_bRunning)
		{
			TRACE_SCOPE("Application Loop");

			PollEvents();

			float deltaTime = CalculateFrameTime();
			Update(deltaTime);
			// This will eventually need to be called after Update,
			// but during the time the render thread is rendering
			g_Engine->BuildRendererData(deltaTime);

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
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		if (RenderAPI::GetCurrent() == ERenderAPI::DX11
#if PLATFORM_SUPPORTS_OPENGL && PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL
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

	void Application::SetImGuiCursor(int32 cursor)
	{
		ImGuiMouseCursor_ cur = (ImGuiMouseCursor_)cursor;
		ECursorType actualCursor = ECursorType::Arrow;
		switch (cur)
		{
			case ImGuiMouseCursor_Arrow:        actualCursor = ECursorType::Arrow; break;
			case ImGuiMouseCursor_TextInput:    actualCursor = ECursorType::TextEdit; break;
			case ImGuiMouseCursor_ResizeAll:    actualCursor = ECursorType::Move; break;
			case ImGuiMouseCursor_ResizeEW:     actualCursor = ECursorType::ResizeWE; break;
			case ImGuiMouseCursor_ResizeNS:     actualCursor = ECursorType::ResizeNS; break;
			case ImGuiMouseCursor_ResizeNESW:   actualCursor = ECursorType::ResizeNESW; break;
			case ImGuiMouseCursor_ResizeNWSE:   actualCursor = ECursorType::ResizeNWSE; break;
			case ImGuiMouseCursor_Hand:         actualCursor = ECursorType::Hand; break;
			case ImGuiMouseCursor_NotAllowed:   actualCursor = ECursorType::Unavailable; break;
		}
		SetCursor(actualCursor);
	}

	void Application::LoadFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		String defaultPath = StringConverter::WStringToString(
			EnginePath::GetFontsPath() + L"OpenSans-Medium.ttf");
		String defaultCondensedPath = StringConverter::WStringToString(
			EnginePath::GetFontsPath() + L"OpenSans_SemiCondensed-Medium.ttf");

		ImFontConfig config;
		config.OversampleH = 2;
		config.OversampleV = 2;
		config.RasterizerMultiply = 1.0f;

		m_Fonts.DefaultFont = io.Fonts->AddFontFromFileTTF(defaultPath.c_str(), 19, &config);
		m_Fonts.DefaultFontCondensed = io.Fonts->AddFontFromFileTTF(defaultCondensedPath.c_str(), 19, &config);
	}

	Application* Application::s_Instance = nullptr;
}
