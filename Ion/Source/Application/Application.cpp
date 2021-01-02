#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"

#define BIND_MEMBER_FUNC(x) std::bind(&x, this, std::placeholders::_1)

namespace Ion {

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
		// Create a platform specific window.
		m_Window = Ion::GenericWindow::Create();

		m_Window->SetEventCallback(BIND_MEMBER_FUNC(Application::OnEvent));

		m_Window->Initialize();
		m_Window->SetTitle(L"Ion Engine");

		m_Window->Show();

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

		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event)
		{
			m_bRunning = false;
			return true;
		});

		m_LayerStack->OnEvent(event);
	}
}
