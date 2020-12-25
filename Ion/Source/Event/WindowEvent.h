#pragma once

#include "Event.h"

namespace Ion
{
	// Window Events
	
	class ION_API WindowEvent : public Event
	{
	public:
		FORCEINLINE int GetWindowHandle() const { return m_WindowHandle; }

		SET_EVENT_CATEGORY(EC_Application | EC_Window)

	protected:
		WindowEvent(int windowHandle) :
			m_WindowHandle(windowHandle) {}

	private:
		int m_WindowHandle;
	};


	class ION_API WindowCloseEvent : public WindowEvent
	{
	public:
		WindowCloseEvent(int windowHandle) :
			WindowEvent(windowHandle) {}

		SET_EVENT_TYPE(WindowClose)
		SET_EVENT_TOSTRING_FORMAT("{window: " << GetWindowHandle() << "}")
	};


	class ION_API WindowResizeEvent : public WindowEvent
	{
	public:
		WindowResizeEvent(int windowHandle, uint width, uint height) :
			WindowEvent(windowHandle),
			m_Width(width),
			m_Height(height) {}

		SET_EVENT_TYPE(WindowResize)
		SET_EVENT_TOSTRING_FORMAT("{window: " << GetWindowHandle() << ", width: " << m_Width << ", height: " << m_Height << "}")

	private:
		uint m_Width, m_Height;
	};


	class ION_API WindowMovedEvent : public WindowEvent
	{
	public:
		WindowMovedEvent(int windowHandle, uint x, uint y) :
			WindowEvent(windowHandle),
			m_X(x),
			m_Y(y) {}

		SET_EVENT_TYPE(WindowMoved)
		SET_EVENT_TOSTRING_FORMAT("{window: " << GetWindowHandle() << ", x: " << m_X << ", y: " << m_Y << "}")

	private:
		uint m_X, m_Y;
	};
}
