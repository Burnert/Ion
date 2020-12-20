#include "Ion.h"

#include <stdio.h>

class IonExample : public Ion::Application
{
public:
	void Run() override
	{
		ION_LOG_TRACE("From application");
		getchar();
	}
};

USE_APPLICATION_CLASS(IonExample)