#pragma once

#include "Application/Event/EventDispatcher.h"

namespace Ion
{
	namespace Mouse
	{
		enum Mouse : uint8
		{
			Invalid         = 0x00,

			Left            = 0x01,
			Right           = 0x02,
			Middle          = 0x03,
			Button4         = 0x04,
			Button5         = 0x05,
		};
	}
	
	namespace Key
	{
		enum Key : uint8
		{
			Invalid         = 0x00,

			Space           = 0x20,
			Minus           = 0x21,
			Equals          = 0x22,
			Comma           = 0x23,
			Period          = 0x24,
			Slash           = 0x25,
			LBracket        = 0x26,
			RBracket        = 0x27,
			Backslash       = 0x28,
			Semicolon       = 0x29,
			Apostrophe      = 0x2A,
			Grave           = 0x2B,

			Zero            = 0x30,
			One             = 0x31,
			Two             = 0x32,
			Three           = 0x33,
			Four            = 0x34,
			Five            = 0x35,
			Six             = 0x36,
			Seven           = 0x37,
			Eight           = 0x38,
			Nine            = 0x39,

			A               = 0x41,
			B               = 0x42,
			C               = 0x43,
			D               = 0x44,
			E               = 0x45,
			F               = 0x46,
			G               = 0x47,
			H               = 0x48,
			I               = 0x49,
			J               = 0x4A,
			K               = 0x4B,
			L               = 0x4C,
			M               = 0x4D,
			N               = 0x4E,
			O               = 0x4F,
			P               = 0x50,
			Q               = 0x51,
			R               = 0x52,
			S               = 0x53,
			T               = 0x54,
			U               = 0x55,
			V               = 0x56,
			W               = 0x57,
			X               = 0x58,
			Y               = 0x59,
			Z               = 0x5A,

			KP_0            = 0x60,
			KP_1            = 0x61,
			KP_2            = 0x62,
			KP_3            = 0x63,
			KP_4            = 0x64,
			KP_5            = 0x65,
			KP_6            = 0x66,
			KP_7            = 0x67,
			KP_8            = 0x68,
			KP_9            = 0x69,
			KP_Multiply     = 0x6A,
			KP_Add          = 0x6B,
			KP_Subtract     = 0x6C,
			KP_Divide       = 0x6D,
			KP_Decimal      = 0x6E,
			KP_Equals       = 0x6F,
			KP_Enter        = 0x70,
			KP_Clear        = 0x71,

			Escape          = 0x80,
			Backspace       = 0x81,
			Tab             = 0x82,
			CapsLock        = 0x83,
			Enter           = 0x84,
			Shift           = 0x85,
			Control         = 0x86,
			Alt             = 0x87,
			PauseBreak      = 0x88,
			PgUp            = 0x89,
			PgDown          = 0x8A,
			Home            = 0x8B,
			End             = 0x8C,
			Insert          = 0x8D,
			Delete          = 0x8E,
			Left            = 0x8F,
			Up              = 0x90,
			Right           = 0x91,
			Down            = 0x92,
			PrintScr        = 0x93,
			Help            = 0x94,
			NumLock         = 0x95,
			ScrollLock      = 0x96,
			LWin            = 0x97,
			RWin            = 0x98,
			Apps            = 0x99,

			F1              = 0xA0,
			F2              = 0xA1,
			F3              = 0xA2,
			F4              = 0xA3,
			F5              = 0xA4,
			F6              = 0xA5,
			F7              = 0xA6,
			F8              = 0xA7,
			F9              = 0xA8,
			F10             = 0xA9,
			F11             = 0xAA,
			F12             = 0xAB,
			F13             = 0xAC,
			F14             = 0xAD,
			F15             = 0xAE,
			F16             = 0xAF,
			F17             = 0xB0,
			F18             = 0xB1,
			F19             = 0xB2,
			F20             = 0xB3,
			F21             = 0xB4,
			F22             = 0xB5,
			F23             = 0xB6,
			F24             = 0xB7,

			LShift          = 0xC0,
			RShift          = 0xC1,
			LControl        = 0xC2,
			RControl        = 0xC3,
			LAlt            = 0xC4,
			RAlt            = 0xC5,
		};
	}

	using MouseButton = Mouse::Mouse;
	using KeyCode = Key::Key;

	enum class MouseInputType : uint8
	{
		Default = 1,
		RawInput = 2,
	};

	class KeyPressedEvent;
	class KeyReleasedEvent;
	class KeyRepeatedEvent;

	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
	class MouseMovedEvent;

	class ION_API InputManager
	{
	public:
		static bool IsKeyPressed(KeyCode keyCode);
		static bool IsKeyRepeated(KeyCode keyCode);
		static bool IsMouseButtonPressed(MouseButton mouseButton);

		static IVector2 GetCursorPosition();

		static std::shared_ptr<InputManager> Create();

		/* Transforms ActualKeyCode to normal KeyCode
		   for LShift returns Shift, etc. */
		static KeyCode TransformKeyCode(KeyCode actualKeyCode);

		FORCEINLINE static MouseInputType GetMouseInputType();

		// @TODO: you know what
		static bool IsRawInputEnabled();

	protected:
		void OnKeyPressedEvent(const KeyPressedEvent& event);
		void OnKeyReleasedEvent(const KeyReleasedEvent& event);
		void OnKeyRepeatedEvent(const KeyRepeatedEvent& event);

		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);

		virtual IVector2 GetCursorPosition_Internal() const = 0;

	protected:
		InputManager();
		virtual ~InputManager() { }

		static uint8 InputPressedFlag;
		static uint8 InputRepeatedFlag;

		template<typename T>
		void DispatchEvent(const T& event)
		{
			TRACE_FUNCTION();
			m_EventDispatcher.Dispatch(event);
		}

	private:
		using InputEventFunctions = TEventFunctionPack<
			TMemberEventFunction<InputManager, KeyPressedEvent,          &OnKeyPressedEvent>,
			TMemberEventFunction<InputManager, KeyReleasedEvent,         &OnKeyReleasedEvent>,
			TMemberEventFunction<InputManager, KeyRepeatedEvent,         &OnKeyRepeatedEvent>,
			TMemberEventFunction<InputManager, MouseButtonPressedEvent,  &OnMouseButtonPressedEvent>,
			TMemberEventFunction<InputManager, MouseButtonReleasedEvent, &OnMouseButtonReleasedEvent>
		>;
		EventDispatcher<InputEventFunctions, InputManager> m_EventDispatcher;

		static std::shared_ptr<InputManager> s_Instance;
		uint8 m_InputStates[256];

		MouseInputType m_MouseInputType;

		friend class Application;
	};

	FORCEINLINE IVector2 InputManager::GetCursorPosition()
	{
		return s_Instance->GetCursorPosition_Internal();
	}

	FORCEINLINE MouseInputType InputManager::GetMouseInputType()
	{
		return s_Instance->m_MouseInputType;
	}

	// @TODO: you know what
	FORCEINLINE bool InputManager::IsRawInputEnabled()
	{
		return true;
	}
}
