#include "IonPCH.h"

#include "GenericWindow.h"
#include "Core/CoreTypes.h"

#include "Log/Logger.h"

namespace Ion
{
	GenericWindow::GenericWindow()
	{
	}

	GenericWindow::~GenericWindow()
	{
	}

	void GenericWindow::Initialize()
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
		ION_LOG_ENGINE_BADPLATFORMFUNCTIONCALL();
		return false;
	}
}
