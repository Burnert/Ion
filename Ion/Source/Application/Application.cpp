#include "IonPCH.h"

#include "Application.h"

#include "Core/Event/InputEvent.h"
#include "Core/Event/EventQueue.h"
#include "Core/Event/EventDispatcher.h"

#define BIND_MEMBER_FUNC(x) std::bind(&x, this, std::placeholders::_1)

namespace Ion {

	Application::Application() :
		m_EventQueue(std::make_unique<EventQueue>())
	{
		m_EventQueue->SetEventHandler(BIND_MEMBER_FUNC(Application::DispatchEvent));
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		// Create a platform specific window.
		m_ApplicationWindow = Ion::GenericWindow::Create();

		m_ApplicationWindow->SetEventCallback(BIND_MEMBER_FUNC(Application::OnEvent));

		m_ApplicationWindow->Initialize();
		m_ApplicationWindow->SetTitle(L"Ion Engine");

		m_ApplicationWindow->Show();

		m_Running = true;
		while (m_Running)
		{
			// Application loop
			PollEvents();
			m_EventQueue->ProcessEvents();
		}
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
			m_Running = false;
			return true;
		});

		dispatcher.Dispatch<WindowMovedEvent>([this](WindowMovedEvent& event)
		{
			return true;
		});

		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& event)
		{
			return true;
		});

		dispatcher.Dispatch<WindowFocusEvent>([this](WindowFocusEvent& event)
		{
			return true;
		});

		dispatcher.Dispatch<WindowLostFocusEvent> ([this](WindowLostFocusEvent& event)
		{
			return true;
		});

		dispatcher.Handled();
	}
}
