#include "IonPCH.h"

#include "Ion.h"

class IonExample : public Ion::Application
{
public:
	IonExample()
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}
};

USE_APPLICATION_CLASS(IonExample)
