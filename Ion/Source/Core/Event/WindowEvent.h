#pragma once

#include "Event.h"

namespace Ion
{
	// Window Events
	
	class ION_API WindowEvent : public Event
	{
	public:
		FORCEINLINE uint64 GetWindowHandle() const { return m_WindowHandle; }

	protected:
		WindowEvent(uint64 windowHandle) :
			m_WindowHandle(windowHandle)
		{
			m_CategoryFlags &= EC_Application | EC_Window;
		}

	private:
		uint64 m_WindowHandle;
	};

	class ION_API WindowCloseEvent : public WindowEvent
	{
	public:
		WindowCloseEvent(uint64 windowHandle) :
			WindowEvent(windowHandle)
		{
			m_Type = EEventType::WindowClose;
			SET_DEBUG_INFO_STRING("WindowCloseEvent: { Handle: %I64u }", GetWindowHandle());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::WindowClose);
	};

	class ION_API WindowResizeEvent : public WindowEvent
	{
	public:
		WindowResizeEvent(uint64 windowHandle, uint32 width, uint32 height) :
			WindowEvent(windowHandle),
			m_Width(width),
			m_Height(height)
		{
			m_Type = EEventType::WindowResize;
			SET_DEBUG_INFO_STRING("WindowResizeEvent: { Handle: %I64u, Width: %u, Height: %u }", GetWindowHandle(), m_Width, m_Height);
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::WindowResize);

		FORCEINLINE uint32 GetWidth() const { return m_Width; }
		FORCEINLINE uint32 GetHeight() const { return m_Height; }

	private:
		uint32 m_Width, m_Height;
	};

	class ION_API WindowMovedEvent : public WindowEvent
	{
	public:
		WindowMovedEvent(uint64 windowHandle, int32 x, int32 y) :
			WindowEvent(windowHandle),
			m_X(x),
			m_Y(y)
		{
			m_Type = EEventType::WindowMoved;
			SET_DEBUG_INFO_STRING("WindowMovedEvent: { Handle: %I64u, X: %u, Y: %u }", GetWindowHandle(), m_X, m_Y);
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::WindowMoved);

		FORCEINLINE uint32 GetX() const { return m_X; }
		FORCEINLINE uint32 GetY() const { return m_Y; }

	private:
		int32 m_X, m_Y;
	};

	class ION_API WindowFocusEvent : public WindowEvent
	{
	public:
		WindowFocusEvent(uint64 windowHandle) :
			WindowEvent(windowHandle)
		{
			m_Type = EEventType::WindowFocus;
			SET_DEBUG_INFO_STRING("WindowFocusEvent: { Handle: %I64u }", GetWindowHandle());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::WindowFocus);
	};

	class ION_API WindowLostFocusEvent : public WindowEvent
	{
	public:
		WindowLostFocusEvent(uint64 windowHandle) :
			WindowEvent(windowHandle)
		{
			m_Type = EEventType::WindowLostFocus;
			SET_DEBUG_INFO_STRING("WindowLostFocusEvent: { Handle: %I64u }", GetWindowHandle());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::WindowLostFocus);
	};

	enum class EDisplayMode : uint8;

	class ION_API WindowChangeDisplayModeEvent : public WindowEvent
	{
	public:
		WindowChangeDisplayModeEvent(uint64 windowHandle, EDisplayMode displayMode) :
			WindowEvent(windowHandle),
			m_DisplayMode(displayMode)
		{
			m_Type = EEventType::WindowChangeDisplayMode;
			SET_DEBUG_INFO_STRING("WindowChangeDisplayModeEvent: { Handle: %I64u, Mode: %u }", GetWindowHandle(), (uint8)m_DisplayMode);
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::WindowChangeDisplayMode);

		FORCEINLINE EDisplayMode GetDisplayMode() { return m_DisplayMode; }

	private:
		EDisplayMode m_DisplayMode;
	};
}
