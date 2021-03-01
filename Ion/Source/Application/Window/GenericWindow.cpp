#include "IonPCH.h"

#include "GenericWindow.h"
#include "Core/CoreTypes.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
	bool GenericWindow::Initialize()
	{
		ION_LOG_ENGINE_CRITICAL("{0} is not implemented!", __FUNCTION__);
		return false;
	}

	WindowDimensions GenericWindow::GetDimensions() const
	{
		return { };
	}

	void GenericWindow::SetEventCallback(EventCallback callback)
	{
		m_EventCallback = callback;
	}
}
