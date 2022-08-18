#include "IonPCH.h"

#include "Error.h"

#include "Core/Logging/Logger.h"

namespace Ion
{
	REGISTER_LOGGER(ErrorLogger, "Core::Error", ELoggerFlags::AlwaysActive);

	void ErrorLoggerInterface::Error(const String& message)
	{
		ErrorLogger.Error(message);
	}

	void ErrorLoggerInterface::Critical(const String& message)
	{
		ErrorLogger.Critical(message);
	}
}
