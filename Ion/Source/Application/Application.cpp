#include "IonPCH.h"

#include "Application.h"
#include "GameLayer.h"
#include "ImGuiLayer.h"

#include "Asset/AssetRegistry.h"

#include "Application/Event/Event.h"
#include "Application/Event/EventQueue.h"
#include "Application/Event/EventDispatcher.h"
#include "Application/Input/Input.h"

#include "Engine/Engine.h"

#include "RHI/RHI.h"
#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

#include "IonApp.h"

namespace Ion
{
	void Application::SetCursor(ECursorType cursor)
	{
		ApplicationLogger.Warn("Changing the cursor is not supported on this platform.");
	}

	ECursorType Application::GetCurrentCursor() const
	{
		return ECursorType::Arrow;
	}

	Renderer* Application::GetRenderer()
	{
		return Renderer::Get();
	}

	Application::Application() :
		m_EventDispatcher(this),
		m_LayerStack(std::make_unique<LayerStack>()),
		m_MainThreadId(std::this_thread::get_id()),
		m_bRunning(true),
		m_Fonts(),
		m_bInFocus(false),
		m_GlobalDeltaTime(0.016f)
	{
		ionassert(g_pClientApplication, "Client application has not been set. ION_DEFINE_MAIN_APPLICATION_CLASS might not have been used.");

		ApplicationLogger.Info("Application has been created.");

		m_EventDispatcher.RegisterEventFunction(&Application::OnWindowCloseEvent);
		m_EventDispatcher.RegisterEventFunction(&Application::OnWindowResizeEvent);
		m_EventDispatcher.RegisterEventFunction(&Application::OnWindowChangeDisplayModeEvent);
		m_EventDispatcher.RegisterEventFunction(&Application::OnWindowLostFocusEvent);
		m_EventDispatcher.RegisterEventFunction(&Application::OnWindowFocusEvent);
		m_EventDispatcher.RegisterEventFunction(&Application::OnKeyPressedEvent);
		m_EventDispatcher.RegisterEventFunction(&Application::OnKeyReleasedEvent);
		m_EventDispatcher.RegisterEventFunction(&Application::OnKeyRepeatedEvent);
	}

	Application::~Application()
	{
		ApplicationLogger.Info("Application has been destroyed.");
	}

	void Application::Init()
	{
		TRACE_FUNCTION();

		ApplicationLogger.Info("Initializing application.");

		PlatformInit();

		EngineTaskQueue::Init();

		// Create a platform specific window.
		m_Window = GenericWindow::Create();
		m_Window->Initialize();

		InputManager::Get()->RegisterRawInputDevices();

		g_Engine->Init();

		// Current thread will render graphics in this window.
		RHI::Create(ERHI::DX11);
		RHI::SetEngineShadersPath(EnginePath::GetShadersPath());
		RHI::Get()->Init(m_Window->GetRHIData()).Unwrap();

		Renderer* renderer = Renderer::Create();
		renderer->Init();
		renderer->SetVSyncEnabled(false);

		AssetRegistry::RegisterEngineVirtualRoots(EnginePath::GetEngineContentPath(), EnginePath::GetShadersPath());
		AssetRegistry::RegisterEngineAssets();

		InitImGui();
		LoadFonts();

		// @TODO: This should be in the Runtime eventually
		//m_LayerStack->PushLayer<GameLayer>("GameLayer");
		//m_LayerStack->PushOverlayLayer<ImGuiLayer>("ImGui");

		SetApplicationTitle(L"Ion");
		SetupWindowTitle();
		m_Window->Show();

		{
			TRACE_SCOPE("Application - Client::OnInit");
			g_pClientApplication->OnInit();
		}
	}

	void Application::RunLoop()
	{
		TRACE_FUNCTION();

		ApplicationLogger.Trace("Starting application loop.");

		// Application loop
		while (m_bRunning)
		{
			TRACE_SCOPE("Application Loop");

			PollEvents();

			m_GlobalDeltaTime = CalculateFrameTime();
			Update(m_GlobalDeltaTime);
			g_pClientApplication->PostUpdate();
			// This will eventually need to be called after Update,
			// but during the time the render thread is rendering
			g_Engine->BuildRendererData(m_GlobalDeltaTime);

			Render();

			m_EventQueue.ProcessEvents([this](const Event& e)
			{
				DispatchEvent(e);
			});

			if (!m_bInFocus)
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(100ms);
			}
		}
	}

	void Application::Shutdown()
	{
		TRACE_FUNCTION();

		ApplicationLogger.Info("Shutting down application.");

		ShutdownImGui();

		{
			TRACE_SCOPE("Application - Client::OnShutdown");
			g_pClientApplication->OnShutdown();
		}

		RHI::Get()->Shutdown();

		g_Engine->Shutdown();

		EngineTaskQueue::Shutdown();

		PlatformShutdown();
	}

	void Application::PostEvent(const Event& e)
	{
		DispatchEvent(e);
	}

	void Application::PostDeferredEvent(const Event& e)
	{
		m_EventQueue.PushEvent(e);
	}

	void Application::DispatchEvent(const Event& e)
	{
		TRACE_FUNCTION();

		m_EventDispatcher.Dispatch(e);

		InputManager::Get()->OnEvent(e);
		m_LayerStack->OnEvent(e);

		{
			TRACE_SCOPE("Application - Client::OnEvent");
			g_pClientApplication->OnEvent(e);
		}
	}

	void Application::Exit()
	{
		// @TODO: Exit
		ApplicationLogger.Critical("TODO: Implement Exit!");
	}

	void Application::Update(float deltaTime)
	{
		TRACE_FUNCTION();

		EngineTaskQueue::Update();

		// Don't reset the cursor if the mouse is being held
		if (!InputManager::IsMouseButtonPressed(EMouse::Left))
		{
			// Reset the cursor
			SetCursor(ECursorType::Arrow);
		}

		g_Engine->Update(deltaTime);

		ImGuiNewFramePlatform();
		ImGui::NewFrame();

		// @TODO: This is broken and kills CPU
		//UpdateWindowTitle(deltaTime);

		//AssetManager::Update();
		
		{
			TRACE_SCOPE("Application - Client::OnUpdate");
			g_pClientApplication->OnUpdate(deltaTime);
		}

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

		RHI::Get()->BeginFrame();
		SetRenderTargetToMainWindow();

		{
			TRACE_SCOPE("Application - Client::OnRender");
			g_pClientApplication->OnRender();
		}

		m_LayerStack->OnRender();

		// Render to the window framebuffer last
		SetRenderTargetToMainWindow();

		{
			TRACE_SCOPE("Render ImGui");
			ImGuiRenderPlatform(ImGui::GetDrawData());
		}

		RHI::Get()->EndFrame(m_Window->GetRHIData());
	}

	void Application::SetRenderTargetToMainWindow()
	{
		WindowDimensions windowSize = GetWindow()->GetDimensions();
		if (windowSize.Width == 0 || windowSize.Height == 0)
		{
			Renderer::Get()->SetRenderTarget(nullptr);
			Renderer::Get()->SetDepthStencil(nullptr);
			return;
		}

		ViewportDescription viewport { };
		viewport.Width = windowSize.Width;
		viewport.Height = windowSize.Height;
		viewport.MaxDepth = 1.0f;
		Renderer::Get()->SetViewport(viewport);
		Renderer::Get()->SetRenderTarget(GetWindow()->GetWindowColorTexture());
		Renderer::Get()->SetDepthStencil(GetWindow()->GetWindowDepthStencilTexture());
	}

	void Application::CallClientAppOnEvent(const Event& e)
	{
		
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
		title += StringConverter::StringToWString(RHI::Get()->GetCurrentDisplayName());
		m_Window->SetTitle(title);
	}

	void Application::UpdateWindowTitle(float deltaTime)
	{
		//wchar fps[30];
		//swprintf_s(fps, L" (%.2f FPS)", 1.0f / deltaTime);
		//m_Window->SetTitle(m_BaseWindowTitle + fps);
	}

	void Application::OnWindowCloseEvent(const WindowCloseEvent& e)
	{
		m_bRunning = false;
	}

	void Application::OnWindowResizeEvent(const WindowResizeEvent& e)
	{
		uint32 width = e.Width;
		uint32 height = e.Height;

		// Cannot create a texture with a width or height of 0.
		// This happens when the window is minimized
		if (width == 0 || height == 0)
			return;

		ApplicationLogger.Trace("Resizing application window buffers to [{}x{}].", width, height);

		//Renderer::Get()->SetViewportDimensions(ViewportDimensions { 0, 0, width, height });
		RHI::Get()->ResizeBuffers(GetWindow()->GetRHIData(), { width, height });
	}

	void Application::OnWindowChangeDisplayModeEvent(const WindowChangeDisplayModeEvent& e)
	{
	}

	void Application::OnWindowLostFocusEvent(const WindowLostFocusEvent& e)
	{
		if (m_Window->GetNativeHandle() == e.WindowHandle)
		{
			m_bInFocus = false;
		}
	}

	void Application::OnWindowFocusEvent(const WindowFocusEvent& e)
	{
		if (m_Window->GetNativeHandle() == e.WindowHandle)
		{
			m_bInFocus = true;
		}
	}

	void Application::OnKeyPressedEvent(const KeyPressedEvent& e)
	{
		// Toggle fullscreen with Alt + Enter
		if (e.KeyCode == EKey::Enter)
		{
			if (InputManager::IsKeyPressed(EKey::LAlt))
			{
				ApplicationLogger.Debug("Fullscreen Toggle");

				bool bFullScreen = GetWindow()->IsFullScreenEnabled();
				GetWindow()->EnableFullScreen(!bFullScreen);
			}
		}
		// Exit application with Alt + F4
		else if (e.KeyCode == EKey::F4)
		{
			if (InputManager::IsKeyPressed(EKey::LAlt))
			{
				Exit();
			}
		}
	}

	void Application::OnKeyReleasedEvent(const KeyReleasedEvent& e)
	{

	}

	void Application::OnKeyRepeatedEvent(const KeyRepeatedEvent& e)
	{

	}

	void Application::InitImGui() const
	{
		TRACE_FUNCTION();

		ApplicationLogger.Info("Initializing ImGui.");

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& imGuiIO = ImGui::GetIO();
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		if (RHI::GetCurrent() == ERHI::DX11
#if PLATFORM_SUPPORTS_OPENGL && PLATFORM_ENABLE_IMGUI_VIEWPORTS_OPENGL
			|| RHI::GetCurrent() == ERHI::OpenGL
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

		ApplicationLogger.Info("Shutting down ImGui.");

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

	static ImFont* LoadFont(const String& fontPath, float size, ImFontConfig* config)
	{
		ImGuiIO& io = ImGui::GetIO();
		String path = EnginePath::GetFontsPath() + fontPath;
		return io.Fonts->AddFontFromFileTTF(path.c_str(), size, config);
	}

	void Application::LoadFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig config;

		m_Fonts.Roboto_14     = LoadFont("Roboto/Roboto-Regular.ttf",         14, &config);
		m_Fonts.Roboto_16     = LoadFont("Roboto/Roboto-Regular.ttf",         16, &config);
		m_Fonts.RobotoSlab_20 = LoadFont("RobotoSlab/RobotoSlab-Regular.ttf", 20, &config);
		m_Fonts.Exo2_20       = LoadFont("Exo2/Exo2-Medium.ttf",              20, &config);
		m_Fonts.Exo2_24       = LoadFont("Exo2/Exo2-Regular.ttf",             24, &config);

		strcpy_s(config.Name, "System Default, 14px");
		String systemFontPath = StringConverter::WStringToString(Platform::GetSystemDefaultFontPath());
		if (!systemFontPath.empty())
		{
			m_Fonts.System_14 = io.Fonts->AddFontFromFileTTF(systemFontPath.c_str(), 14, &config);
		}

		io.FontDefault = m_Fonts.Roboto_14;
	}
}
