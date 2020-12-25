#include "IonPCH.h"

#include "Application.h"

#include "Event/InputEvent.h"
#include "Event/EventQueue.h"

namespace Ion {

	Application::Application() :
		m_EventQueue(std::make_unique<EventQueue>()) 
	{
		Logger::Init();
	}

	Application::~Application() {}

	void Application::Init()
	{
		// Create a platform specific window.
		m_ApplicationWindow = Ion::GenericWindow::Create();

		m_ApplicationWindow->Initialize();
		m_ApplicationWindow->SetTitle(L"Ion Engine");

		m_ApplicationWindow->Show();

		Ion::KeyPressedEvent key(65, 0);
		Ion::MouseMovedEvent mouse(500, 200);

		ION_LOG_ENGINE_DEBUG("Pushing events to queue.");
		m_EventQueue->PushEvent(key);
		m_EventQueue->PushEvent(mouse);

		m_Running = true;
		while (m_Running)
		{
			// Application loop
			PollEvents();
			m_EventQueue->ProcessEvents();
		}
	}

	void Application::PollEvents()
	{
	}
}
