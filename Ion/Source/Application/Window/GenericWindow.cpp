#include "IonPCH.h"

#include "GenericWindow.h"
#include "Application/Application.h"
#include "Core/CoreTypes.h"

namespace Ion
{
	GenericWindow::GenericWindow() :
		m_bCursorLocked(false),
		m_bCursorShown(true)
	{ }

	bool GenericWindow::Initialize()
	{
		ApplicationLogger.Critical("{0} is not implemented!", __FUNCTION__);
		return false;
	}

	bool GenericWindow::Initialize(const std::shared_ptr<GenericWindow>& parentWindow)
	{
		ApplicationLogger.Critical("{0} is not implemented!", __FUNCTION__);
		return false;
	}

	WindowDimensions GenericWindow::GetDimensions() const
	{
		return { 0, 0 };
	}

	bool GenericWindow::IsFullScreenEnabled() const
	{
		return false;
	}

	bool GenericWindow::IsInFocus() const
	{
		return false;
	}

	Vector2 GenericWindow::GetCenterPosition() const
	{
		WindowDimensions dimensions = GetDimensions();
		return Vector2 { dimensions.Width * 0.5f, dimensions.Height * 0.5f };
	}

	void* GenericWindow::GetNativeHandle() const
	{
		return nullptr;
	}
}
