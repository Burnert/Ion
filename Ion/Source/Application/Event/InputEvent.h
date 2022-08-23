#pragma once

#include "Event.h"

namespace Ion
{
	// Keyboard Events

	class ION_API KeyboardEvent : public Event
	{
	public:
		FORCEINLINE uint32 GetKeyCode() const { return m_KeyCode; }
		FORCEINLINE uint32 GetActualKeyCode() const { return m_ActualKeyCode; }

	protected:
		KeyboardEvent(uint32 keyCode, uint32 actualKeyCode) :
			m_KeyCode(keyCode),
			m_ActualKeyCode(actualKeyCode)
		{
			m_CategoryFlags &= EC_Input | EC_Keyboard;
		}

	private:
		uint32 m_KeyCode;
		uint32 m_ActualKeyCode;
	};

	class ION_API KeyPressedEvent : public KeyboardEvent
	{
	public:
		KeyPressedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(keyCode, actualKeyCode)
		{
			m_Type = EEventType::KeyPressed;
			SET_DEBUG_INFO_STRING("KeyPressedEvent: { KeyCode: %x, ActualKeyCode: %x }", GetKeyCode(), GetActualKeyCode());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::KeyPressed);
	};

	class ION_API KeyReleasedEvent : public KeyboardEvent
	{
	public:
		KeyReleasedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(keyCode, actualKeyCode) 
		{
			m_Type = EEventType::KeyReleased;
			SET_DEBUG_INFO_STRING("KeyReleasedEvent: { KeyCode: %x, ActualKeyCode: %x }", GetKeyCode(), GetActualKeyCode());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::KeyReleased);
	};

	class ION_API KeyRepeatedEvent : public KeyboardEvent
	{
	public:
		KeyRepeatedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(keyCode, actualKeyCode)
		{
			m_Type = EEventType::KeyRepeated;
			SET_DEBUG_INFO_STRING("KeyRepeatedEvent: { KeyCode: %x, ActualKeyCode: %x }", GetKeyCode(), GetActualKeyCode());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::KeyRepeated);
	};

	// Mouse Events

	class ION_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y, int32 ssx, int32 ssy) :
			m_X(x), m_Y(y), m_SSX(ssx), m_SSY(ssy)
		{
			m_CategoryFlags &= EC_Input | EC_Mouse;
			m_Type = EEventType::MouseMoved;
			SET_DEBUG_INFO_STRING("MouseMovedEvent: { X: %.2f, Y: %.2f, SSX: %.2i, SSY: %.2i }", m_X, m_Y, m_SSX, m_SSY);
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::MouseMoved);

		FORCEINLINE float GetX() const { return m_X; }
		FORCEINLINE float GetY() const { return m_Y; }
		FORCEINLINE int32 GetScreenX() const { return m_SSX; }
		FORCEINLINE int32 GetScreenY() const { return m_SSY; }

	private:
		float m_X, m_Y;
		int32 m_SSX, m_SSY;
	};

	class ION_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float offset) :
			m_Offset(offset)
		{
			m_CategoryFlags &= EC_Input | EC_Mouse;
			m_Type = EEventType::MouseScrolled;
			SET_DEBUG_INFO_STRING("MouseScrolledEvent: { Offset: %.2f }", m_Offset);
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::MouseScrolled);

		FORCEINLINE float GetOffset() { return m_Offset; }

	private:
		float m_Offset;
	};

	// Mouse Button Events

	class ION_API MouseButtonEvent : public Event
	{
	public:
		FORCEINLINE uint32 GetMouseButton() const { return m_Button; }

	protected:
		MouseButtonEvent(uint32 button) :
			m_Button(button)
		{
			m_CategoryFlags &= EC_Input | EC_Mouse | EC_MouseButton;
		}
		
	private:
		uint32 m_Button;
	};

	class ION_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(uint32 button) :
			MouseButtonEvent(button)
		{
			m_Type = EEventType::MouseButtonPressed;
			SET_DEBUG_INFO_STRING("MouseButtonPressedEvent: { Button: %u }", GetMouseButton());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::MouseButtonPressed);
	};

	class ION_API MouseDoubleClickEvent : public MouseButtonEvent
	{
	public:
		MouseDoubleClickEvent(uint32 button) :
			MouseButtonEvent(button)
		{
			m_Type = EEventType::MouseDoubleClick;
			SET_DEBUG_INFO_STRING("MouseDoubleClickEvent: { Button: %u }", GetMouseButton());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::MouseDoubleClick);
	};

	class ION_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(uint32 button) :
			MouseButtonEvent(button)
		{
			m_Type = EEventType::MouseButtonReleased;
			SET_DEBUG_INFO_STRING("MouseButtonReleasedEvent: { Button: %u }", GetMouseButton());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::MouseButtonReleased);
	};

	// -------------------
	// Raw Input ---------

	// Mouse events

	class ION_API RawInputMouseMovedEvent : public MouseMovedEvent
	{
	public:
		RawInputMouseMovedEvent(float x, float y) :
			MouseMovedEvent(x, y, 0, 0)
		{
			m_CategoryFlags &= EC_RawInput;
			m_Type = EEventType::RawInputMouseMoved;
			SET_DEBUG_INFO_STRING("RawInputMouseMovedEvent: { X: %.2f, Y: %.2f }", GetX(), GetY());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::RawInputMouseMoved);
	};

	class ION_API RawInputMouseScrolledEvent : public MouseScrolledEvent
	{
	public:
		RawInputMouseScrolledEvent(float offset) :
			MouseScrolledEvent(offset)
		{
			m_CategoryFlags &= EC_RawInput;
			m_Type = EEventType::RawInputMouseScrolled;
			SET_DEBUG_INFO_STRING("RawInputMouseScrolledEvent: { Offset: %.2f }", GetOffset());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::RawInputMouseScrolled);
	};

	// Mouse Button Events

	class ION_API RawInputMouseButtonPressedEvent : public MouseButtonPressedEvent
	{
	public:
		RawInputMouseButtonPressedEvent(uint32 button) :
			MouseButtonPressedEvent(button)
		{
			m_CategoryFlags &= EC_RawInput;
			m_Type = EEventType::RawInputMouseButtonPressed;
			SET_DEBUG_INFO_STRING("RawInputMouseButtonPressedEvent: { Button: %u }", GetMouseButton());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::RawInputMouseButtonPressed);
	};

	class ION_API RawInputMouseButtonReleasedEvent : public MouseButtonReleasedEvent
	{
	public:
		RawInputMouseButtonReleasedEvent(uint32 button) :
			MouseButtonReleasedEvent(button)
		{
			m_CategoryFlags &= EC_RawInput;
			m_Type = EEventType::RawInputMouseButtonReleased;
			SET_DEBUG_INFO_STRING("RawInputMouseButtonReleasedEvent: { Button: %u }", GetMouseButton());
		}

		STATIC_EVENT_TYPE_GETTER(EEventType::RawInputMouseButtonReleased);
	};
}
