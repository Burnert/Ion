#pragma once

#include "Core/Platform/Windows/WindowsHeaders.h"
#include "Core/CoreApi.h"
#include "Application/Input/Input.h"

namespace Ion
{
	class ION_API WindowsInputManager : public InputManager
	{
	protected:
		virtual IVector2 GetCursorPosition_Internal() const override;

		/* Translates a Windows key code to internal Ion key code.
		   When the key code is invalid (0) it returns false. */
		static bool TranslateWindowsKeyCode(uint32* keyCodePtr);

		/* Translates an internal Ion key code back to a Windows virtual key */
		static inline uint32 TranslateWindowsKeyCodeReverse(uint16 internalKeyCode)
		{
			// This is too slow
			// Should be another LUT
			// but I don't have time for it now.
			// @TODO: Make a LUT in the future.
			for (int32 i = 0; i < 256; ++i)
			{
				if (s_InputKeyCodeLookup[i] == internalKeyCode)
					return s_InputKeyCodeLookup[i];
			}
			return 0;
		}

		static constexpr uint8 s_InputKeyCodeLookup[256] = {
			// 0x00 ---------------------------
			0x00, // 0x00 : Invalid keycode
			Mouse::Left,
			Mouse::Right,
			0x00, // 0x03 : VK_CANCEL
			Mouse::Middle,
			Mouse::Button4,
			Mouse::Button5,
			0x00,
			Key::Backspace,
			Key::Tab,
			0x00, 0x00,
			Key::KP_Clear, // 0x0C : VK_CLEAR
			Key::Enter,
			0x00, 0x00,
			// 0x10 ---------------------------
			Key::Shift,
			Key::Control,
			Key::Alt,
			Key::PauseBreak,
			Key::CapsLock,
			// 0x15 - 0x1A : IME Keys
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			Key::Escape,
			// 0x1C - 0x1F : IME Keys
			0x00, 0x00, 0x00, 0x00,
			// 0x20 ---------------------------
			Key::Space,
			Key::PgUp,
			Key::PgDown,
			Key::End,
			Key::Home,
			Key::Left,
			Key::Up,
			Key::Right,
			Key::Down,
			0x00, // 0x29 : VK_SELECT
			0x00, // 0x2A : VK_PRINT
			0x00, // 0x2B : VK_EXECUTE
			Key::PrintScr,
			Key::Insert,
			Key::Delete,
			Key::Help,
			// 0x30 ---------------------------
			// 0x30 - 0x39 : Numbers - same as ASCII
			Key::Zero,
			Key::One,
			Key::Two,
			Key::Three,
			Key::Four,
			Key::Five,
			Key::Six,
			Key::Seven,
			Key::Eight,
			Key::Nine,
			// 0x3A = 0x40 : unassigned
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			// 0x41 - 0x5A : Letters - same as ASCII
			Key::A,
			Key::B,
			Key::C,
			Key::D,
			Key::E,
			Key::F,
			Key::G,
			Key::H,
			Key::I,
			Key::J,
			Key::K,
			Key::L,
			Key::M,
			Key::N,
			Key::O,
			Key::P,
			Key::Q,
			Key::R,
			Key::S,
			Key::T,
			Key::U,
			Key::V,
			Key::W,
			Key::X,
			Key::Y,
			Key::Z,
			// 0x5B ---------------------------
			Key::LWin,
			Key::RWin,
			Key::Apps,
			0x00,
			0x00, // 0x5F : VK_SLEEP
			// 0x60 ---------------------------
			// 0x60 - 0x6F : Keypad
			Key::KP_0,
			Key::KP_1,
			Key::KP_2,
			Key::KP_3,
			Key::KP_4,
			Key::KP_5,
			Key::KP_6,
			Key::KP_7,
			Key::KP_8,
			Key::KP_9,
			Key::KP_Multiply,
			Key::KP_Add,
			Key::KP_Decimal, // 0x6C : VK_SEPARATOR
			Key::KP_Subtract,
			Key::KP_Decimal,
			Key::KP_Divide,
			// 0x70 ---------------------------
			// 0x70 - 0x87 : Function keys
			Key::F1,
			Key::F2,
			Key::F3,
			Key::F4,
			Key::F5,
			Key::F6,
			Key::F7,
			Key::F8,
			Key::F9,
			Key::F10,
			Key::F11,
			Key::F12,
			Key::F13,
			Key::F14,
			Key::F15,
			Key::F16,
			Key::F17,
			Key::F18,
			Key::F19,
			Key::F20,
			Key::F21,
			Key::F22,
			Key::F23,
			Key::F24,
			// 0x88 - 0x8F : UI navigation
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			// 0x90 ---------------------------
			Key::NumLock,
			Key::ScrollLock,
			Key::KP_Equals, // 0x92 : VK_OEM_NEC_EQUAL
			0x00, 0x00, 0x00, 0x00,
			// 0x97 - 0x9F : unassigned
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			// 0x90 ---------------------------
			Key::LShift,
			Key::RShift,
			Key::LControl,
			Key::RControl,
			Key::LAlt,
			Key::RAlt,
			// 0xA6 - 0xAC : Browser keys
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00,
			// 0xAD - 0xB7 : Media keys
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00,
			// 0xB8
			0x00, 0x00,
			// OEM keys
			Key::Semicolon,
			Key::Equals,
			Key::Comma,
			Key::Minus,
			Key::Period,
			Key::Slash,
			Key::Grave,
			0x00, 0x00,
			// 0xC3 - 0xDA : Gamepad input
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			Key::LBracket,
			Key::Backslash,
			Key::RBracket,
			Key::Apostrophe,
			0x00,
			// 0xE0 ---------------------------
			// 0xE0 - 0xFF
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};

		friend class WindowsApplication;
		friend class WindowsWindow;
	};
}
