#pragma once

#include "Core/Core.h"
#include "Event/InputEvent.h"
#include "Event/WindowEvent.h"
#include "Application/Window/GenericWindow.h"

// Specifies the main class of the application (can be used only once)
#define USE_APPLICATION_CLASS(name) \
Ion::Application* Ion::CreateApplication() \
{ \
	return new name; \
}

namespace Ion {

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
		virtual void EventHandler(Event& event);

		virtual bool OnWindowCloseEvent(WindowCloseEvent& event);
		virtual bool OnWindowMovedEvent(WindowMovedEvent& event);
		virtual bool OnWindowResizeEvent(WindowResizeEvent& event);
		virtual bool OnWindowFocusEvent(WindowFocusEvent& event);
		virtual bool OnWindowLostFocusEvent(WindowLostFocusEvent& event);

	private:
		bool m_Running = false;

		std::unique_ptr<class EventQueue> m_EventQueue;
		std::shared_ptr<GenericWindow> m_ApplicationWindow;
	};

	Application* CreateApplication();
}
