#pragma once

#include "Event.h"

namespace Ion
{
	// Keyboard Events

	class ION_API KeyboardEvent : public Event
	{
	public:
		FORCEINLINE uint GetKeyCode() const { return m_KeyCode; }
		FORCEINLINE uint GetActualKeyCode() const { return m_ActualKeyCode; }

		SET_EVENT_CATEGORY(EC_Input | EC_Keyboard)

	protected:
		KeyboardEvent(uint keyCode, uint actualKeyCode) :
			m_KeyCode(keyCode),
			m_ActualKeyCode(actualKeyCode) {}

	private:
		uint m_KeyCode;
		uint m_ActualKeyCode;
	};


	class ION_API KeyPressedEvent : public KeyboardEvent
	{
	public:
		KeyPressedEvent(uint keyCode, uint actualKeyCode, uint repeatCount) :
			KeyboardEvent(keyCode, actualKeyCode),
			m_RepeatCount(repeatCount) {}

		FORCEINLINE uint GetRepeatCount() const { return m_RepeatCount; }

		SET_EVENT_TYPE(KeyPressed)
		SET_EVENT_TOSTRING_FORMAT("{ keyCode: " << GetKeyCode() << ", actualKeyCode: " << GetActualKeyCode() << " } (repeat " << m_RepeatCount << ")")

	private:
		uint m_RepeatCount;
	};


	class ION_API KeyReleasedEvent : public KeyboardEvent
	{
	public:
		KeyReleasedEvent(uint keyCode, uint actualKeyCode) :
			KeyboardEvent(keyCode, actualKeyCode) {}

		SET_EVENT_TYPE(KeyReleased)
		SET_EVENT_TOSTRING_FORMAT("{ keyCode: " << GetKeyCode() << ", actualKeyCode: " << GetActualKeyCode() << " }")
	};

	// Mouse Events

	class ION_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) :
			m_X(x), m_Y(y) {}

		FORCEINLINE float GetX() const { return m_X; }
		FORCEINLINE float GetY() const { return m_Y; }

		SET_EVENT_TYPE(MouseMoved)
		SET_EVENT_CATEGORY(EC_Input | EC_Mouse)
		SET_EVENT_TOSTRING_FORMAT("{ x: " << m_X << ", y: " << m_Y << " }")

	private:
		float m_X, m_Y;
	};


	class ION_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float offset) :
			m_Offset(offset) {}

		FORCEINLINE float GetOffset() { return m_Offset; }

		SET_EVENT_TYPE(MouseScrolled)
		SET_EVENT_CATEGORY(EC_Input | EC_Mouse)
		SET_EVENT_TOSTRING_FORMAT("{ offset: " << m_Offset << " }")

	private:
		float m_Offset;
	};

	// Mouse Button Events

	class ION_API MouseButtonEvent : public Event
	{
	public:
		FORCEINLINE uint GetMouseButton() const { return m_Button; }

		SET_EVENT_CATEGORY(EC_Input | EC_Mouse | EC_MouseButton)

	protected:
		MouseButtonEvent(uint button) :
			m_Button(button) {}
		
	private:
		uint m_Button;
	};


	class ION_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(uint button) :
			MouseButtonEvent(button) {}

		SET_EVENT_TYPE(MouseButtonPressed)
		SET_EVENT_TOSTRING_FORMAT("{ button: " << GetMouseButton() << " }")
	};


	class ION_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(uint button) :
			MouseButtonEvent(button) {}

		SET_EVENT_TYPE(MouseButtonPressed)
		SET_EVENT_TOSTRING_FORMAT("{ button: " << GetMouseButton() << " }")
	};
}
