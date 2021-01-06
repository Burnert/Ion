#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"
#include "Core/Input/Input.h"


namespace Ion
{
	DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop, "Main Loop", "Game");
	DECLARE_PERFORMANCE_COUNTER(Counter_MainLoop_Section, "Main Loop Section", "Game");

	Application* Application::Get()
	{
		// This goes off when Application was not yet created.
		// (shouldn't ever happen, unless you're doing something weird)
		ASSERT(s_Instance);
		return s_Instance;
	}

	Application::Application() :
		m_EventQueue(std::make_unique<EventQueue>()),
		m_LayerStack(std::make_unique<LayerStack>())
	{
		m_EventQueue->SetEventHandler(BIND_MEMBER_FUNC(Application::DispatchEvent));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		m_InputManager = InputManager::Get();

		// Create a platform specific window.
		m_Window = GenericWindow::Create();

		m_Window->SetEventCallback(BIND_MEMBER_FUNC(Application::OnEvent));

		m_Window->Initialize();
		m_Window->SetTitle(L"Ion Engine");

		m_Window->Show();

		m_bRunning = true;
		while (m_bRunning)
		{
			{
				SCOPED_PERFORMANCE_COUNTER(Counter_MainLoop);

				// Application loop
				PollEvents();

				using namespace std::chrono_literals;
				std::this_thread::sleep_for(0.2s);

				MANUAL_PERFORMANCE_COUNTER(Counter_MainLoop_Section);
				Counter_MainLoop_Section->Start();

				std::this_thread::sleep_for(0.2s);

				Counter_MainLoop_Section->Stop();

				Update(0.0f);
				Render();

				m_EventQueue->ProcessEvents();
			}

			COUNTER_TIME_INFO(dataMainLoop, "Counter_MainLoop");
			COUNTER_TIME_INFO(dataMainLoopSection, "Counter_MainLoop_Section");

			LOG_WARN("{0} Data: {1}", dataMainLoop.Name, dataMainLoop.GetTimeNs());
			LOG_WARN("{0} Data: {1}", dataMainLoopSection.Name, dataMainLoopSection.GetTimeNs());
		}
	}

	void Application::PollEvents()
	{
		// Platform specific
	}

	void Application::Update(float DeltaTime)
	{
		m_LayerStack->OnUpdate(DeltaTime);
	}

	void Application::Render()
	{
		m_LayerStack->OnRender();
	}

	void Application::OnEvent(Event& event)
	{
		if (event.IsDeferred())
			m_EventQueue->PushEvent(event.MakeShared());
		else
			DispatchEvent(event);
	}

	void Application::DispatchEvent(Event& event)
	{
		ION_LOG_ENGINE_DEBUG("Event: {0}", event.Debug_ToString());

		EventDispatcher dispatcher(event);

		// Window events

		dispatcher.Dispatch<WindowCloseEvent>(
			[this](WindowCloseEvent& event)
			{
				m_bRunning = false;
				return false;
			});

		m_InputManager->OnEvent(event);
		m_LayerStack->OnEvent(event);
	}

	Application* Application::s_Instance = nullptr;
}
