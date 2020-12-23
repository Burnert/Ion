#include "Ion.h"

#include <stdio.h>
#include <sstream>

class IonExample : public Ion::Application
{
public:
	void Run() override
	{
		ION_LOG_TRACE("Application running");
		getchar();
	}
};

USE_APPLICATION_CLASS(IonExample)
