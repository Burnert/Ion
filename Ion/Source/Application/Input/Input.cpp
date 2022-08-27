#include "IonPCH.h"

#include "Input.h"
#include "Application/Event/InputEvent.h"

namespace Ion
{
	uint8 InputManager::InputPressedFlag = Bitflag(0);
	uint8 InputManager::InputRepeatedFlag = Bitflag(1);

	KeyCode InputManager::TransformKeyCode(KeyCode actualKeyCode)
	{
		switch (actualKeyCode)
		{
		case Key::LShift:
		case Key::RShift:
			return Key::Shift;
		case Key::LAlt:
		case Key::RAlt:
			return Key::Alt;
		case Key::LControl:
		case Key::RControl:
			return Key::Control;
		default:
			return actualKeyCode;
		}
	}

	InputManager::InputManager() :
		m_EventDispatcher(this),
		// Use Raw Input by default
		m_MouseInputType(MouseInputType::RawInput)
	{
		memset(m_InputStates, 0, sizeof(m_InputStates));

		m_EventDispatcher.RegisterEventFunction(&InputManager::OnKeyPressedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnKeyReleasedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnKeyRepeatedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnMouseButtonPressedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnMouseButtonReleasedEvent);
	}

	bool InputManager::IsKeyPressed(KeyCode keyCode)
	{
		// @TODO: This is getting bugged when the program freezes
		// It's because this is event based and the program doesn't receive KeyUp events
		return (bool)(s_Instance->m_InputStates[keyCode] & InputPressedFlag);
	}

	bool InputManager::IsKeyRepeated(KeyCode keyCode)
	{
		return (bool)(s_Instance->m_InputStates[keyCode] & InputRepeatedFlag);
	}

	bool InputManager::IsMouseButtonPressed(MouseButton mouseButton)
	{
		return (bool)(s_Instance->m_InputStates[mouseButton] & InputPressedFlag);
	}

	void InputManager::OnKeyPressedEvent(const KeyPressedEvent& event)
	{
		KeyCode actualKeyCode = (KeyCode)event.GetActualKeyCode();
		// Set states on receiving event
		uint8* keyPtr = &m_InputStates[actualKeyCode];
		SetBitflags(*keyPtr, InputPressedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), update it too.
		KeyCode keyCode = (KeyCode)event.GetKeyCode();
		if (keyCode != actualKeyCode)
		{
			keyPtr = &m_InputStates[keyCode];
			SetBitflags(*keyPtr, InputPressedFlag);
		}
	}

	void InputManager::OnKeyReleasedEvent(const KeyReleasedEvent& event)
	{
		KeyCode actualKeyCode = (KeyCode)event.GetActualKeyCode();
		// Unset states on receiving event
		uint8* keyPtr = &m_InputStates[actualKeyCode];
		UnsetBitflags(*keyPtr, InputPressedFlag);
		UnsetBitflags(*keyPtr, InputRepeatedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), unset it if the other actual
		// key is not pressed.
		KeyCode keyCode = (KeyCode)event.GetKeyCode();
		if (keyCode != actualKeyCode)
		{
			if (actualKeyCode == Key::LShift   && !IsKeyPressed(Key::RShift)   ||
				actualKeyCode == Key::RShift   && !IsKeyPressed(Key::LShift)   ||
				actualKeyCode == Key::LAlt     && !IsKeyPressed(Key::RAlt)     ||
				actualKeyCode == Key::RAlt     && !IsKeyPressed(Key::LAlt)     ||
				actualKeyCode == Key::LControl && !IsKeyPressed(Key::RControl) ||
				actualKeyCode == Key::RControl && !IsKeyPressed(Key::LControl))
			{
				keyPtr = &m_InputStates[keyCode];
				UnsetBitflags(*keyPtr, InputPressedFlag);
				UnsetBitflags(*keyPtr, InputRepeatedFlag);
			}
		}
	}

	void InputManager::OnKeyRepeatedEvent(const KeyRepeatedEvent& event)
	{
		KeyCode actualKeyCode = (KeyCode)event.GetActualKeyCode();
		// Set states on receiving event
		uint8* keyPtr = &m_InputStates[actualKeyCode];
		SetBitflags(*keyPtr, InputRepeatedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), update it too.
		KeyCode keyCode = (KeyCode)event.GetKeyCode();
		if (keyCode != actualKeyCode)
		{
			keyPtr = &m_InputStates[keyCode];
			SetBitflags(*keyPtr, InputRepeatedFlag);
		}
	}

	void InputManager::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
	{
		MouseButton buttonCode = (MouseButton)event.GetMouseButton();
		// Mouse states are stored in the same array as key states
		uint8* statePtr = &m_InputStates[buttonCode];
		SetBitflags(*statePtr, InputPressedFlag);
	}

	void InputManager::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
	{
		MouseButton buttonCode = (MouseButton)event.GetMouseButton();
		// Mouse states are stored in the same array as key states
		uint8* statePtr = &m_InputStates[buttonCode];
		UnsetBitflags(*statePtr, InputPressedFlag);
	}

	void InputManager::OnEvent(const Event& e)
	{
		TRACE_FUNCTION();

		m_EventDispatcher.Dispatch(e);
	}

	std::shared_ptr<InputManager> InputManager::s_Instance = nullptr;
}
