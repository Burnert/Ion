#include "Ion.h"

#include <stdio.h>

class IonExample : public Ion::Application
{
public:
	void Run() override
	{
		printf("Hello World!");
		getchar();
	}
};

USE_APPLICATION_CLASS(IonExample)