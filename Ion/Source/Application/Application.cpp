#include "Application.h"

#include "Event/InputEvent.h"

namespace Ion {

	Application::Application()
	{
		ION_LOG_ENGINE_TRACE("From engine");

		MouseButtonPressedEvent e(1);
		ION_LOG_ENGINE_INFO(e.Debug_ToString());
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
	}
}