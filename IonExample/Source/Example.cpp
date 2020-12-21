#include "Ion.h"

#include <stdio.h>

class IonExample : public Ion::Application
{
public:
	void Run() override
	{
		ION_LOG_TRACE("From application");

		Ion::MouseMovedEvent e(720, 576);
		ION_LOG_INFO(e.Debug_ToString());

		getchar();
	}
};

USE_APPLICATION_CLASS(IonExample)
