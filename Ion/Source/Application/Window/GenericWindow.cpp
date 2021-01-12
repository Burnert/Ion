#include "IonPCH.h"

#include "GenericWindow.h"
#include "Core/CoreTypes.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
	GenericWindow::GenericWindow()
	{
	}

	GenericWindow::~GenericWindow()
	{
	}

	bool GenericWindow::Initialize()
	{
		ION_LOG_ENGINE_CRITICAL("{0} is not implemented!", __FUNCTION__);
		return false;
	}

	void GenericWindow::Show()
	{
	}

	void GenericWindow::Hide()
	{
	}

	void GenericWindow::SetTitle(const WCStr title)
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
		ION_LOG_ENGINE_BADPLATFORMFUNCTIONCALL();
		return false;
	}
	void GenericWindow::SetEventCallback(EventCallback callback)
	{
		m_EventCallback = callback;
	}
}
