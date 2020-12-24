#pragma once

#include "Core/Core.h"
#include "Event/InputEvent.h"

// Specifies the main class of the application (can be used only once)
#define USE_APPLICATION_CLASS(name) \
Ion::Application* Ion::CreateApplication() \
{ \
	return new name; \
}

namespace Ion {

	class ION_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Init();

	private:
		bool m_Running = false;

		std::unique_ptr<class EventQueue> m_EventQueue;
	};

	Application* CreateApplication();
}
