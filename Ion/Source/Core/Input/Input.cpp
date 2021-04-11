#include "IonPCH.h"

#include "Input.h"
#include "Core/CoreUtility.h"

namespace Ion
{
	ubyte InputManager::InputPressedFlag = Bitflag(0);
	ubyte InputManager::InputRepeatedFlag = Bitflag(1);

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
		// Use Raw Input by default
		m_MouseInputType(MouseInputType::RawInput)
	{
		memset(m_InputStates, 0, sizeof(m_InputStates));
	}

	bool InputManager::IsKeyPressed(KeyCode keyCode)
	{
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

	void InputManager::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<KeyPressedEvent>(BIND_METHOD_1P(InputManager::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_METHOD_1P(InputManager::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyRepeatedEvent>(BIND_METHOD_1P(InputManager::OnKeyRepeatedEvent));

		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_METHOD_1P(InputManager::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_METHOD_1P(InputManager::OnMouseButtonReleasedEvent));
	}

	bool InputManager::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		KeyCode actualKeyCode = (KeyCode)event.GetActualKeyCode();
		// Set states on receiving event
		ubyte* keyPtr = &m_InputStates[actualKeyCode];
		SetBitflags(*keyPtr, InputPressedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), update it too.
		KeyCode keyCode = (KeyCode)event.GetKeyCode();
		if (keyCode != actualKeyCode)
		{
			keyPtr = &m_InputStates[keyCode];
			SetBitflags(*keyPtr, InputPressedFlag);
		}

		return false;
	}

	bool InputManager::OnKeyReleasedEvent(KeyReleasedEvent& event)
	{
		KeyCode actualKeyCode = (KeyCode)event.GetActualKeyCode();
		// Unset states on receiving event
		ubyte* keyPtr = &m_InputStates[actualKeyCode];
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

		return false;
	}

	bool InputManager::OnKeyRepeatedEvent(KeyRepeatedEvent& event)
	{
		KeyCode actualKeyCode = (KeyCode)event.GetActualKeyCode();
		// Set states on receiving event
		ubyte* keyPtr = &m_InputStates[actualKeyCode];
		SetBitflags(*keyPtr, InputRepeatedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), update it too.
		KeyCode keyCode = (KeyCode)event.GetKeyCode();
		if (keyCode != actualKeyCode)
		{
			keyPtr = &m_InputStates[keyCode];
			SetBitflags(*keyPtr, InputRepeatedFlag);
		}

		return false;
	}

	bool InputManager::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		MouseButton buttonCode = (MouseButton)event.GetMouseButton();
		// Mouse states are stored in the same array as key states
		ubyte* statePtr = &m_InputStates[buttonCode];
		SetBitflags(*statePtr, InputPressedFlag);

		return false;
	}

	bool InputManager::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
	{
		MouseButton buttonCode = (MouseButton)event.GetMouseButton();
		// Mouse states are stored in the same array as key states
		ubyte* statePtr = &m_InputStates[buttonCode];
		UnsetBitflags(*statePtr, InputPressedFlag);

		return false;
	}

	TShared<InputManager> InputManager::s_Instance = nullptr;
}
