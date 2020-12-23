#include "Application.h"

#include "Event/InputEvent.h"

namespace Ion {

	Application::Application()
	{
		Ion::KeyPressedEvent key(65, 0);
		Ion::MouseMovedEvent mouse(500, 200);

		ION_LOG_ENGINE_INFO(key.Debug_ToString());
		ION_LOG_ENGINE_INFO(mouse.Debug_ToString());
	}

	Application::~Application()
	{
	}
}