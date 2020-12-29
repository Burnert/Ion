#pragma once

#include "Core/Core.h"
#include "Core/Event/InputEvent.h"
#include "Core/Event/WindowEvent.h"
#include "Application/Window/GenericWindow.h"
#include "Application/Layer/LayerStack.h"

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
	public:
		using EventPtr = std::shared_ptr<Event>;

		Application();
		virtual ~Application();

		void Init();

	protected:
		// Platform specific method for polling application events / messages.
		virtual void PollEvents();

		virtual void Update(float DeltaTime);
		virtual void Render();

		virtual void OnEvent(Event& event);
		virtual void DispatchEvent(Event& event);

	private:
		bool m_bRunning = false;

		std::unique_ptr<EventQueue> m_EventQueue;
		std::shared_ptr<GenericWindow> m_ApplicationWindow;
		
		std::unique_ptr<LayerStack> m_LayerStack;
	};

	Application* CreateApplication();
}
