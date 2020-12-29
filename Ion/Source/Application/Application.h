#pragma once

#include "Core/Core.h"
#include "Core/Event/InputEvent.h"
#include "Core/Event/WindowEvent.h"
#include "Application/Window/GenericWindow.h"

// Specifies the main class of the application (can be used only once)
#define USE_APPLICATION_CLASS(name) \
Ion::Application* Ion::CreateApplication() \
{ \
	return new name; \
}

namespace Ion {

	class EventQueue;

	class ION_API Application
	{
		using EventPtr = std::shared_ptr<Event>;
	public:
		Application();
		virtual ~Application();

		void Init();

	protected:
		// Platform specific method for polling application events / messages.
		virtual void PollEvents() { };

		virtual void OnEvent(EventPtr event);
		virtual void DispatchEvent(Event& event);

	private:
		bool m_Running = false;

		std::unique_ptr<EventQueue> m_EventQueue;
		std::shared_ptr<GenericWindow> m_ApplicationWindow;
		
	};

	Application* CreateApplication();
}
