#include "IonPCH.h"

#include "Application/Application.h"
#include "Application/Input/Input.h"
#include "Core/Platform/Windows/WindowsCore.h"

namespace Ion
{
	constexpr TFixedArray<uint8, 256> GetWindowsToInternalKeyCodeLUT()
	{
		TFixedArray<uint8, 256> lut { };

		// Mouse
		lut[VK_LBUTTON]  = EMouse::Left;
		lut[VK_RBUTTON]  = EMouse::Right;
		lut[VK_MBUTTON]  = EMouse::Middle;
		lut[VK_XBUTTON1] = EMouse::Button4;
		lut[VK_XBUTTON2] = EMouse::Button5;

		// Action Keys
		lut[VK_BACK]     = EKey::Backspace;
		lut[VK_TAB]      = EKey::Tab;
		lut[VK_CLEAR]    = EKey::KP_Clear;
		lut[VK_RETURN]   = EKey::Enter;
		lut[VK_SHIFT]    = EKey::Shift;
		lut[VK_CONTROL]  = EKey::Control;
		lut[VK_MENU]     = EKey::Alt;
		lut[VK_PAUSE]    = EKey::PauseBreak;
		lut[VK_CAPITAL]  = EKey::CapsLock;
		lut[VK_ESCAPE]   = EKey::Escape;
		lut[VK_SPACE]    = EKey::Space;
		lut[VK_PRIOR]    = EKey::PgUp;
		lut[VK_NEXT]     = EKey::PgDown;
		lut[VK_END]      = EKey::End;
		lut[VK_HOME]     = EKey::Home;
		lut[VK_LEFT]     = EKey::Left;
		lut[VK_UP]       = EKey::Up;
		lut[VK_RIGHT]    = EKey::Right;
		lut[VK_DOWN]     = EKey::Down;
		lut[VK_SNAPSHOT] = EKey::PrintScr;
		lut[VK_INSERT]   = EKey::Insert;
		lut[VK_DELETE]   = EKey::Delete;
		lut[VK_HELP]     = EKey::Help;
		lut[VK_LWIN]     = EKey::LWin;
		lut[VK_RWIN]     = EKey::RWin;
		lut[VK_APPS]     = EKey::Apps;

		// ASCII
		lut['0'] = EKey::D0;
		lut['1'] = EKey::D1;
		lut['2'] = EKey::D2;
		lut['3'] = EKey::D3;
		lut['4'] = EKey::D4;
		lut['5'] = EKey::D5;
		lut['6'] = EKey::D6;
		lut['7'] = EKey::D7;
		lut['8'] = EKey::D8;
		lut['9'] = EKey::D9;
		lut['A'] = EKey::A;
		lut['B'] = EKey::B;
		lut['C'] = EKey::C;
		lut['D'] = EKey::D;
		lut['E'] = EKey::E;
		lut['F'] = EKey::F;
		lut['G'] = EKey::G;
		lut['H'] = EKey::H;
		lut['I'] = EKey::I;
		lut['J'] = EKey::J;
		lut['K'] = EKey::K;
		lut['L'] = EKey::L;
		lut['M'] = EKey::M;
		lut['N'] = EKey::N;
		lut['O'] = EKey::O;
		lut['P'] = EKey::P;
		lut['Q'] = EKey::Q;
		lut['R'] = EKey::R;
		lut['S'] = EKey::S;
		lut['T'] = EKey::T;
		lut['U'] = EKey::U;
		lut['V'] = EKey::V;
		lut['W'] = EKey::W;
		lut['Y'] = EKey::X;
		lut['X'] = EKey::Y;
		lut['Z'] = EKey::Z;

		// Keypad
		lut[VK_NUMPAD0]   = EKey::KP_0;
		lut[VK_NUMPAD1]   = EKey::KP_1;
		lut[VK_NUMPAD2]   = EKey::KP_2;
		lut[VK_NUMPAD3]   = EKey::KP_3;
		lut[VK_NUMPAD4]   = EKey::KP_4;
		lut[VK_NUMPAD5]   = EKey::KP_5;
		lut[VK_NUMPAD6]   = EKey::KP_6;
		lut[VK_NUMPAD7]   = EKey::KP_7;
		lut[VK_NUMPAD8]   = EKey::KP_8;
		lut[VK_NUMPAD9]   = EKey::KP_9;
		lut[VK_MULTIPLY]  = EKey::KP_Multiply;
		lut[VK_ADD]       = EKey::KP_Add;
		lut[VK_SEPARATOR] = EKey::KP_Decimal;
		lut[VK_SUBTRACT]  = EKey::KP_Subtract;
		lut[VK_DECIMAL]   = EKey::KP_Decimal;
		lut[VK_DIVIDE]    = EKey::KP_Divide;

		// F Keys
		lut[VK_F1]  = EKey::F1;
		lut[VK_F2]  = EKey::F2;
		lut[VK_F3]  = EKey::F3;
		lut[VK_F4]  = EKey::F4;
		lut[VK_F5]  = EKey::F5;
		lut[VK_F6]  = EKey::F6;
		lut[VK_F7]  = EKey::F7;
		lut[VK_F8]  = EKey::F8;
		lut[VK_F9]  = EKey::F9;
		lut[VK_F10] = EKey::F10;
		lut[VK_F11] = EKey::F11;
		lut[VK_F12] = EKey::F12;
		lut[VK_F13] = EKey::F13;
		lut[VK_F14] = EKey::F14;
		lut[VK_F15] = EKey::F15;
		lut[VK_F16] = EKey::F16;
		lut[VK_F17] = EKey::F17;
		lut[VK_F18] = EKey::F18;
		lut[VK_F19] = EKey::F19;
		lut[VK_F20] = EKey::F20;
		lut[VK_F21] = EKey::F21;
		lut[VK_F22] = EKey::F22;
		lut[VK_F23] = EKey::F23;
		lut[VK_F24] = EKey::F24;

		// -Lock Keys
		lut[VK_NUMLOCK] = EKey::NumLock;
		lut[VK_SCROLL]  = EKey::ScrollLock;

		// L/R Modifier Keys
		lut[VK_LSHIFT]   = EKey::LShift;
		lut[VK_RSHIFT]   = EKey::RShift;
		lut[VK_LCONTROL] = EKey::LControl;
		lut[VK_RCONTROL] = EKey::RControl;
		lut[VK_LMENU]    = EKey::LAlt;
		lut[VK_RMENU]    = EKey::RAlt;
		
		// OEM Keys
		lut[VK_OEM_NEC_EQUAL] = EKey::KP_Equals;
		lut[VK_OEM_1]         = EKey::Semicolon;
		lut[VK_OEM_PLUS]      = EKey::Equals;
		lut[VK_OEM_COMMA]     = EKey::Comma;
		lut[VK_OEM_MINUS]     = EKey::Minus;
		lut[VK_OEM_PERIOD]    = EKey::Period;
		lut[VK_OEM_2]         = EKey::Slash;
		lut[VK_OEM_3]         = EKey::Grave;
		lut[VK_OEM_4]         = EKey::LBracket;
		lut[VK_OEM_5]         = EKey::Backslash;
		lut[VK_OEM_6]         = EKey::RBracket;
		lut[VK_OEM_7]         = EKey::Apostrophe;

		return lut;
	}

	constexpr TFixedArray<uint8, 256> s_WindowsToInternalKeyCodeLUT = GetWindowsToInternalKeyCodeLUT();

	constexpr TFixedArray<uint8, 256> GetInternalToWindowsKeyCodeLUT()
	{
		TFixedArray<uint8, 256> lut { };

		for (uint32 iNative = 0; iNative < 256; ++iNative)
		{
			if (uint8 code = s_WindowsToInternalKeyCodeLUT[iNative])
			{
				// @TODO: Handle the VK_SEPARATOR / VK_DECIMAL ambiguity.
				// if (i == VK_SEPARATOR)

				lut[code] = iNative;
			}
		}

		return lut;
	}

	constexpr TFixedArray<uint8, 256> s_InternalToWindowsKeyCodeLUT = GetInternalToWindowsKeyCodeLUT();

	Result<void, PlatformError> InputManager::RegisterRawInputDevices_Native()
	{
		RAWINPUTDEVICE mouseRid;
		// Mouse:
		mouseRid.usUsagePage = 0x01;
		mouseRid.usUsage = 0x02;
		mouseRid.dwFlags = 0;
		mouseRid.hwndTarget = (HWND)Application::Get()->GetWindow()->GetNativeHandle();

		if (!::RegisterRawInputDevices(&mouseRid, 1, sizeof(mouseRid)))
		{
			ionthrow(PlatformError, "Cannot register Raw Input devices.\n{}", Windows::GetLastErrorMessage());
		}

		return Ok();
	}

	IVector2 InputManager::GetCursorPosition_Native()
	{
		POINT pos;
		::GetCursorPos(&pos);
		return IVector2(pos.x, pos.y);
	}

	/**
	 * Translates a Windows key code to internal Ion key code.
	 * When the key code is invalid (0) it returns false.
	 */
	bool InputManager::TranslateNativeKeyCode_Native(uint32* nativeKeyCode)
	{
		ionassert(nativeKeyCode);
		ionassert(*nativeKeyCode <= TNumericLimits<uint8>::max());

		if (uint8 translated = s_WindowsToInternalKeyCodeLUT[*nativeKeyCode])
		{
			*nativeKeyCode = translated;
			return true;
		}
		return false;
	}

	bool InputManager::TranslateInternalKeyCode_Native(uint32* internalKeyCode)
	{
		ionassert(internalKeyCode);
		ionassert(*internalKeyCode <= TNumericLimits<uint8>::max());

		if (uint8 translated = s_InternalToWindowsKeyCodeLUT[*internalKeyCode])
		{
			*internalKeyCode = translated;
			return true;
		}
		return false;
	}
}
