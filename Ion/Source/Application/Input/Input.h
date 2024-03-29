#pragma once

#include "Application/Event/EventDispatcher.h"

namespace Ion
{
	REGISTER_LOGGER(InputLogger, "Application::Input");

	namespace EMouse
	{
		enum Type : uint8
		{
			Invalid         = 0x00,

			Left            = 0x01,
			Right           = 0x02,
			Middle          = 0x03,
			Button4         = 0x04,
			Button5         = 0x05,
		};
	}
	
	namespace EKey
	{
		enum Type : uint8
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

			D0              = 0x30,
			D1              = 0x31,
			D2              = 0x32,
			D3              = 0x33,
			D4              = 0x34,
			D5              = 0x35,
			D6              = 0x36,
			D7              = 0x37,
			D8              = 0x38,
			D9              = 0x39,

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

	enum class EMouseInputType : uint8
	{
		Default  = 0,
		RawInput = 1,
	};

	class ION_API InputManager
	{
	public:
		static InputManager* Get();

		static bool IsKeyPressed(EKey::Type keyCode);
		static bool IsKeyRepeated(EKey::Type keyCode);
		static bool IsMouseButtonPressed(EMouse::Type mouseButton);

		static IVector2 GetCursorPosition();

		/* Transforms ActualKeyCode to normal KeyCode
		   for LShift returns Shift, etc. */
		static EKey::Type TransformKeyCode(EKey::Type actualKeyCode);

		static EMouseInputType GetMouseInputType();
		static bool IsRawInputEnabled();
		static bool IsRawInputAvailable();

		static bool TranslateNativeKeyCode(uint32* nativeKeyCode);
		static bool TranslateInternalKeyCode(uint32* internalKeyCode);

	protected:
		InputManager();

		void RegisterRawInputDevices();
		void OnEvent(const Event& e);

		static uint8 InputPressedFlag;
		static uint8 InputRepeatedFlag;

	private:
		static Result<void, PlatformError> RegisterRawInputDevices_Native();

		void OnKeyPressedEvent(const KeyPressedEvent& e);
		void OnKeyReleasedEvent(const KeyReleasedEvent& e);
		void OnKeyRepeatedEvent(const KeyRepeatedEvent& e);

		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& e);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& e);

		static IVector2 GetCursorPosition_Native();
		static bool TranslateNativeKeyCode_Native(uint32* nativeKeyCode);
		static bool TranslateInternalKeyCode_Native(uint32* internalKeyCode);

	private:
		TEventDispatcher<InputManager> m_EventDispatcher;

		TFixedArray<uint8, 256> m_InputStates;
		EMouseInputType m_MouseInputType;
		bool m_bRawInputAvailable;

		static InputManager* s_Instance;

		friend class Application;
	};

	FORCEINLINE InputManager* InputManager::Get()
	{
		if (!s_Instance)
			s_Instance = new InputManager;
		return s_Instance;
	}

	FORCEINLINE IVector2 InputManager::GetCursorPosition()
	{
		return GetCursorPosition_Native();
	}

	FORCEINLINE EMouseInputType InputManager::GetMouseInputType()
	{
		return Get()->m_MouseInputType;
	}

	FORCEINLINE bool InputManager::IsRawInputEnabled()
	{
		return Get()->m_MouseInputType == EMouseInputType::RawInput;
	}

	FORCEINLINE bool InputManager::IsRawInputAvailable()
	{
		return Get()->m_bRawInputAvailable;
	}

	FORCEINLINE bool InputManager::TranslateNativeKeyCode(uint32* nativeKeyCode)
	{
		return TranslateNativeKeyCode_Native(nativeKeyCode);
	}

	FORCEINLINE bool InputManager::TranslateInternalKeyCode(uint32* internalKeyCode)
	{
		return TranslateInternalKeyCode_Native(internalKeyCode);
	}
}
