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

		SET_EVENT_CATEGORY(EC_Input | EC_Keyboard)

	protected:
		KeyboardEvent(uint32 keyCode, uint32 actualKeyCode) :
			m_KeyCode(keyCode),
			m_ActualKeyCode(actualKeyCode) {}

	private:
		uint32 m_KeyCode;
		uint32 m_ActualKeyCode;
	};


	class ION_API KeyPressedEvent : public KeyboardEvent
	{
	public:
		KeyPressedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(keyCode, actualKeyCode)
		{ }

		SET_EVENT_TYPE(KeyPressed)
		SET_EVENT_TOSTRING_FORMAT("{ keyCode: " << std::hex << GetKeyCode() << ", actualKeyCode: " << GetActualKeyCode() << " }")
	};


	class ION_API KeyReleasedEvent : public KeyboardEvent
	{
	public:
		KeyReleasedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(keyCode, actualKeyCode) 
		{ }

		SET_EVENT_TYPE(KeyReleased)
		SET_EVENT_TOSTRING_FORMAT("{ keyCode: " << std::hex << GetKeyCode() << ", actualKeyCode: " << GetActualKeyCode() << " }")
	};

	class ION_API KeyRepeatedEvent : public KeyboardEvent
	{
	public:
		KeyRepeatedEvent(uint32 keyCode, uint32 actualKeyCode) :
			KeyboardEvent(keyCode, actualKeyCode)
		{ }

		SET_EVENT_TYPE(KeyRepeated)
		SET_EVENT_TOSTRING_FORMAT("{ keyCode: " << std::hex << GetKeyCode() << ", actualKeyCode: " << GetActualKeyCode() << " }")
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
		FORCEINLINE uint32 GetMouseButton() const { return m_Button; }

		SET_EVENT_CATEGORY(EC_Input | EC_Mouse | EC_MouseButton)

	protected:
		MouseButtonEvent(uint32 button) :
			m_Button(button) {}
		
	private:
		uint32 m_Button;
	};


	class ION_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(uint32 button) :
			MouseButtonEvent(button) {}

		SET_EVENT_TYPE(MouseButtonPressed)
		SET_EVENT_TOSTRING_FORMAT("{ button: " << GetMouseButton() << " }")
	};


	class ION_API MouseDoubleClickEvent : public MouseButtonEvent
	{
	public:
		MouseDoubleClickEvent(uint32 button) :
			MouseButtonEvent(button) {}

		SET_EVENT_TYPE(MouseDoubleClick)
		SET_EVENT_TOSTRING_FORMAT("{ button: " << GetMouseButton() << " }")
	};


	class ION_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(uint32 button) :
			MouseButtonEvent(button) {}

		SET_EVENT_TYPE(MouseButtonReleased)
		SET_EVENT_TOSTRING_FORMAT("{ button: " << GetMouseButton() << " }")
	};

	// -------------------
	// Raw Input ---------


	// Mouse events

	class ION_API RawInputMouseMovedEvent : public MouseMovedEvent
	{
	public:
		RawInputMouseMovedEvent(float x, float y) :
			MouseMovedEvent(x, y) { }

		SET_EVENT_TYPE(RawInputMouseMoved)
		SET_EVENT_CATEGORY(EC_Input | EC_Mouse | EC_RawInput)
	};


	class ION_API RawInputMouseScrolledEvent : public MouseScrolledEvent
	{
	public:
		RawInputMouseScrolledEvent(float offset) :
			MouseScrolledEvent(offset) { }

		SET_EVENT_TYPE(RawInputMouseScrolled)
		SET_EVENT_CATEGORY(EC_Input | EC_Mouse | EC_RawInput)
	};

	// Mouse Button Events

	class ION_API RawInputMouseButtonPressedEvent : public MouseButtonPressedEvent
	{
	public:
		RawInputMouseButtonPressedEvent(uint32 button) :
			MouseButtonPressedEvent(button) { }

		SET_EVENT_TYPE(RawInputMouseButtonPressed)
		SET_EVENT_CATEGORY(EC_Input | EC_Mouse | EC_MouseButton | EC_RawInput)
	};


	class ION_API RawInputMouseButtonReleasedEvent : public MouseButtonReleasedEvent
	{
	public:
		RawInputMouseButtonReleasedEvent(uint32 button) :
			MouseButtonReleasedEvent(button) { }

		SET_EVENT_TYPE(RawInputMouseButtonReleased)
		SET_EVENT_CATEGORY(EC_Input | EC_Mouse | EC_MouseButton | EC_RawInput)
	};
}
