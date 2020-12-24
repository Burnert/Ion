#include "IonPCH.h"

#include "GenericWindow.h"
#include "Core/CoreTypes.h"

#include "Log/Logger.h"

static CStr _badFunctionCallMessage = "{0} is not supposed to be called on this platform!";

namespace Ion
{
	GenericWindow::GenericWindow()
	{
	}

	GenericWindow::~GenericWindow()
	{
	}

	void GenericWindow::SetTitle(WCStr title)
	{
	}

	void GenericWindow::SetEnabled(bool bEnabled)
	{
	}

	void GenericWindow::Resize()
	{
	}

	bool GenericWindow::GetDimensions(int& width, int& height) const
	{
		ION_LOG_ENGINE_CRITICAL(_badFunctionCallMessage, __FUNCTION__);
		return false;
	}
}
