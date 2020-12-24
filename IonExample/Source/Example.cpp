#include "IonPCH.h"

#include "Ion.h"

class IonExample : public IonApplication
{
public:
	IonExample()
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}
};

USE_APPLICATION_CLASS(IonExample)
