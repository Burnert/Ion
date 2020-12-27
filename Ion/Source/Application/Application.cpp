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

	void Application::OnEvent(EventPtr event)
	{
		m_EventQueue->PushEvent(event);
	}

	void Application::EventHandler(Event& event)
	{
		ION_LOG_ENGINE_DEBUG("Event: {0}", event.Debug_ToString());

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>     (BIND_MEMBER_FUNC(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowMovedEvent>     (BIND_MEMBER_FUNC(Application::OnWindowMovedEvent));
		dispatcher.Dispatch<WindowResizeEvent>    (BIND_MEMBER_FUNC(Application::OnWindowResizeEvent));
		dispatcher.Dispatch<WindowFocusEvent>     (BIND_MEMBER_FUNC(Application::OnWindowFocusEvent));
		dispatcher.Dispatch<WindowLostFocusEvent> (BIND_MEMBER_FUNC(Application::OnWindowLostFocusEvent));
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent& event)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowMovedEvent(WindowMovedEvent& event)
	{
		return true;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& event)
	{
		return true;
	}

	bool Application::OnWindowFocusEvent(WindowFocusEvent& event)
	{
		return true;
	}

	bool Application::OnWindowLostFocusEvent(WindowLostFocusEvent& event)
	{
		return true;
	}
}
