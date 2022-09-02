#include "IonPCH.h"

#include "Input.h"
#include "Application/Event/Event.h"

namespace Ion
{
	uint8 InputManager::InputPressedFlag  = 1 << 0;
	uint8 InputManager::InputRepeatedFlag = 1 << 1;

	EKey::Type InputManager::TransformKeyCode(EKey::Type actualKeyCode)
	{
		switch (actualKeyCode)
		{
		case EKey::LShift:
		case EKey::RShift:
			return EKey::Shift;
		case EKey::LAlt:
		case EKey::RAlt:
			return EKey::Alt;
		case EKey::LControl:
		case EKey::RControl:
			return EKey::Control;
		default:
			return actualKeyCode;
		}
	}

	InputManager::InputManager() :
		m_EventDispatcher(this),
		m_MouseInputType(EMouseInputType::RawInput),
		m_bRawInputAvailable(true),
		m_InputStates()
	{
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnKeyPressedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnKeyReleasedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnKeyRepeatedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnMouseButtonPressedEvent);
		m_EventDispatcher.RegisterEventFunction(&InputManager::OnMouseButtonReleasedEvent);
	}

	bool InputManager::IsKeyPressed(EKey::Type keyCode)
	{
		// @TODO: This is getting bugged when the program freezes
		// It's because this is event based and the program doesn't receive KeyUp events
		return (bool)(Get()->m_InputStates[keyCode] & InputPressedFlag);
	}

	bool InputManager::IsKeyRepeated(EKey::Type keyCode)
	{
		return (bool)(Get()->m_InputStates[keyCode] & InputRepeatedFlag);
	}

	bool InputManager::IsMouseButtonPressed(EMouse::Type mouseButton)
	{
		return (bool)(Get()->m_InputStates[mouseButton] & InputPressedFlag);
	}

	void InputManager::OnKeyPressedEvent(const KeyPressedEvent& e)
	{
		EKey::Type actualKeyCode = (EKey::Type)e.ActualKeyCode;
		// Set states on receiving event
		uint8* keyPtr = &m_InputStates[actualKeyCode];
		SetBitflags(*keyPtr, InputPressedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), update it too.
		EKey::Type keyCode = (EKey::Type)e.KeyCode;
		if (keyCode != actualKeyCode)
		{
			keyPtr = &m_InputStates[keyCode];
			SetBitflags(*keyPtr, InputPressedFlag);
		}
	}

	void InputManager::OnKeyReleasedEvent(const KeyReleasedEvent& e)
	{
		EKey::Type actualKeyCode = (EKey::Type)e.ActualKeyCode;
		// Unset states on receiving event
		uint8* keyPtr = &m_InputStates[actualKeyCode];
		UnsetBitflags(*keyPtr, InputPressedFlag);
		UnsetBitflags(*keyPtr, InputRepeatedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), unset it if the other actual
		// key is not pressed.
		EKey::Type keyCode = (EKey::Type)e.KeyCode;
		if (keyCode != actualKeyCode)
		{
			if (actualKeyCode == EKey::LShift   && !IsKeyPressed(EKey::RShift)   ||
				actualKeyCode == EKey::RShift   && !IsKeyPressed(EKey::LShift)   ||
				actualKeyCode == EKey::LAlt     && !IsKeyPressed(EKey::RAlt)     ||
				actualKeyCode == EKey::RAlt     && !IsKeyPressed(EKey::LAlt)     ||
				actualKeyCode == EKey::LControl && !IsKeyPressed(EKey::RControl) ||
				actualKeyCode == EKey::RControl && !IsKeyPressed(EKey::LControl))
			{
				keyPtr = &m_InputStates[keyCode];
				UnsetBitflags(*keyPtr, InputPressedFlag);
				UnsetBitflags(*keyPtr, InputRepeatedFlag);
			}
		}
	}

	void InputManager::OnKeyRepeatedEvent(const KeyRepeatedEvent& e)
	{
		EKey::Type actualKeyCode = (EKey::Type)e.ActualKeyCode;
		// Set states on receiving event
		uint8* keyPtr = &m_InputStates[actualKeyCode];
		SetBitflags(*keyPtr, InputRepeatedFlag);

		// If the normal key code is different than actual
		// (Shift, Alt, etc.), update it too.
		EKey::Type keyCode = (EKey::Type)e.KeyCode;
		if (keyCode != actualKeyCode)
		{
			keyPtr = &m_InputStates[keyCode];
			SetBitflags(*keyPtr, InputRepeatedFlag);
		}
	}

	void InputManager::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& e)
	{
		EMouse::Type buttonCode = (EMouse::Type)e.Button;
		// Mouse states are stored in the same array as key states
		uint8* statePtr = &m_InputStates[buttonCode];
		SetBitflags(*statePtr, InputPressedFlag);
	}

	void InputManager::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& e)
	{
		EMouse::Type buttonCode = (EMouse::Type)e.Button;
		// Mouse states are stored in the same array as key states
		uint8* statePtr = &m_InputStates[buttonCode];
		UnsetBitflags(*statePtr, InputPressedFlag);
	}

	void InputManager::OnEvent(const Event& e)
	{
		TRACE_FUNCTION();

		m_EventDispatcher.Dispatch(e);
	}

	void InputManager::RegisterRawInputDevices()
	{
		TRACE_FUNCTION();

		RegisterRawInputDevices_Native()
			.Err([this](Error& err)
			{
				InputLogger.Warn("Cannot register raw input devices. Default input mode will be used instead.\n{}", err.Message);
				m_MouseInputType = EMouseInputType::Default;
				m_bRawInputAvailable = false;
			})
			.Ok([this]
			{
				InputLogger.Info("Raw input devices have been registered successfully.");
				m_bRawInputAvailable = true;
			});
	}

	InputManager* InputManager::s_Instance = nullptr;
}
