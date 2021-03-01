#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Input/Input.h"

#include "Core/Platform/PlatformCore.h"

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
		m_MainThreadId(std::this_thread::get_id())
	{
		m_EventQueue->SetEventHandler(BIND_METHOD_1P(Application::DispatchEvent));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		m_InputManager = InputManager::Create();

		// Create a platform specific window.
		m_Window = GenericWindow::Create();

		m_Window->SetEventCallback(BIND_METHOD_1P(Application::OnEvent));

		m_Window->Initialize();
		m_Window->SetTitle(L"Ion Engine");

		// Current thread will render graphics in this window.
		m_Window->MakeRenderingContextCurrent();

		m_Window->Show();

		// Call client overriden Init function
		OnInit();

		RunMainLoop();

		// Call client overriden Shutdown function
		OnShutdown();
	}

	void Application::PollEvents()
	{
		// Platform specific
	}

	void Application::Update(float DeltaTime)
	{
		m_LayerStack->OnUpdate(DeltaTime);
		OnUpdate(DeltaTime);
	}

	void Application::Render()
	{
		m_LayerStack->OnRender();
		OnRender();
	}

	void Application::OnEvent(Event& event)
	{
		if (event.IsDeferred())
			m_EventQueue->PushEvent(event.AsShared());
		else
			DispatchEvent(event);
	}

	void Application::DispatchEvent(Event& event)
	{
		ION_LOG_ENGINE_DEBUG("Event: {0}", event.Debug_ToString());

		EventDispatcher dispatcher(event);

		// Handle close event in application
		dispatcher.Dispatch<WindowCloseEvent>(
			[this](WindowCloseEvent& event)
			{
				m_bRunning = false;
				return false;
			});

		m_InputManager->OnEvent(event);
		m_LayerStack->OnEvent(event);
		OnClientEvent(event);
	}

	void Application::RunMainLoop()
	{
		m_bRunning = true;
		while (m_bRunning)
		{
			// Application loop
			PollEvents();

			Update(0.0f);
			Render();

			m_EventQueue->ProcessEvents();
		}
	}

	Application* Application::s_Instance = nullptr;
}
