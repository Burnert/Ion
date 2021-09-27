#pragma once

#include "Event.h"

namespace Ion
{
	// Window Events
	
	class ION_API WindowEvent : public Event
	{
	public:
		FORCEINLINE uint64 GetWindowHandle() const { return m_WindowHandle; }

		SET_EVENT_CATEGORY(EC_Application | EC_Window)

	protected:
		WindowEvent(uint64 windowHandle) :
			m_WindowHandle(windowHandle) { }

	private:
		uint64 m_WindowHandle;
	};


	class ION_API WindowCloseEvent : public WindowEvent
	{
	public:
		WindowCloseEvent(uint64 windowHandle) :
			WindowEvent(windowHandle) { }

		SET_EVENT_TYPE(WindowClose)
		SET_EVENT_TOSTRING_FORMAT("{ window: " << GetWindowHandle() << " }")
	};


	class ION_API WindowResizeEvent : public WindowEvent
	{
	public:
		WindowResizeEvent(uint64 windowHandle, uint32 width, uint32 height) :
			WindowEvent(windowHandle),
			m_Width(width),
			m_Height(height) { }

		FORCEINLINE uint32 GetWidth() const { return m_Width; }
		FORCEINLINE uint32 GetHeight() const { return m_Height; }

		SET_EVENT_TYPE(WindowResize)
		SET_EVENT_TOSTRING_FORMAT("{ window: " << GetWindowHandle() << ", width: " << m_Width << ", height: " << m_Height << " }")

	private:
		uint32 m_Width, m_Height;
	};


	class ION_API WindowMovedEvent : public WindowEvent
	{
	public:
		WindowMovedEvent(uint64 windowHandle, int32 x, int32 y) :
			WindowEvent(windowHandle),
			m_X(x),
			m_Y(y) { }

		FORCEINLINE uint32 GetX() const { return m_X; }
		FORCEINLINE uint32 GetY() const { return m_Y; }

		SET_EVENT_TYPE(WindowMoved)
		SET_EVENT_TOSTRING_FORMAT("{ window: " << GetWindowHandle() << ", x: " << m_X << ", y: " << m_Y << " }")

	private:
		int32 m_X, m_Y;
	};


	class ION_API WindowFocusEvent : public WindowEvent
	{
	public:
		WindowFocusEvent(uint64 windowHandle) :
			WindowEvent(windowHandle) { }

		SET_EVENT_TYPE(WindowFocus)
		SET_EVENT_TOSTRING_FORMAT("{ window: " << GetWindowHandle() << " }")
	};


	class ION_API WindowLostFocusEvent : public WindowEvent
	{
	public:
		WindowLostFocusEvent(uint64 windowHandle) :
			WindowEvent(windowHandle) { }

		SET_EVENT_TYPE(WindowLostFocus)
		SET_EVENT_TOSTRING_FORMAT("{ window: " << GetWindowHandle() << " }")
	};

	enum class EDisplayMode : uint8;

	class ION_API WindowChangeDisplayModeEvent : public WindowEvent
	{
	public:
		WindowChangeDisplayModeEvent(uint64 windowHandle, EDisplayMode displayMode) :
			WindowEvent(windowHandle),
			m_DisplayMode(displayMode) { }

		FORCEINLINE EDisplayMode GetDisplayMode() { return m_DisplayMode; }

		SET_EVENT_TYPE(WindowChangeDisplayMode);
		SET_EVENT_TOSTRING_FORMAT("{ window: " << GetWindowHandle() << ", mode: " << (uint8)m_DisplayMode << " }");

	private:
		EDisplayMode m_DisplayMode;
	};
}
