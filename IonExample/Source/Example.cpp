#include "Ion.h"

#include <stdio.h>

class IonExample : public Ion::Application
{
public:
	IonExample()
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}
};

USE_APPLICATION_CLASS(IonExample)
