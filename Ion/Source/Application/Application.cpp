#include "IonPCH.h"

#include "Application.h"

#include "Event/InputEvent.h"
#include "Event/EventQueue.h"

#define BIND_MEMBER_FUNC(x) std::bind(&x, this, std::placeholders::_1)

namespace Ion {

	Application::Application() :
		m_EventQueue(std::make_unique<EventQueue>())
	{
		m_EventQueue->SetEventHandler(BIND_MEMBER_FUNC(Application::EventHandler));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		// Create a platform specific window.
		m_ApplicationWindow = Ion::GenericWindow::Create();

		m_ApplicationWindow->Initialize();
		m_ApplicationWindow->SetTitle(L"Ion Engine");

		m_ApplicationWindow->SetEventCallback(BIND_MEMBER_FUNC(Application::OnEvent));

		m_ApplicationWindow->Show();

		m_Running = true;
		while (m_Running)
		{
			// Application loop
			PollEvents();
			m_EventQueue->ProcessEvents();
		}
	}

	void Application::OnEvent(std::shared_ptr<Event> event)
	{
		m_EventQueue->PushEvent(event);
	}

	void Application::EventHandler(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_MEMBER_FUNC(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_MEMBER_FUNC(Application::OnWindowResizeEvent));
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent& event)
	{
		ION_LOG_ENGINE_DEBUG("Close event: {0}", event.Debug_ToString());
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& event)
	{
		return true;
	}
}
