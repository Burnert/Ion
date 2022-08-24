#include "IonPCH.h"

#include "WindowsInput.h"

namespace Ion
{
	std::shared_ptr<InputManager> InputManager::Create()
	{
		if (s_Instance == nullptr)
			s_Instance = std::make_shared<WindowsInputManager>();

		return s_Instance;
	}

	IVector2 WindowsInputManager::GetCursorPosition_Internal() const
	{
		POINT pos;
		::GetCursorPos(&pos);
		return IVector2(pos.x, pos.y);
	}

	bool WindowsInputManager::TranslateWindowsKeyCode(uint32* keyCodePtr)
	{
		uint32& keyCode = *keyCodePtr;
		*keyCodePtr = s_InputKeyCodeLookup[keyCode];
		return (bool)(*keyCodePtr);
	}
}