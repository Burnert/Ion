#include "IonPCH.h"

#include "WindowsInput.h"

namespace Ion
{
	TShared<InputManager> InputManager::Create()
	{
		if (s_Instance == nullptr)
			s_Instance = MakeShared<WindowsInputManager>();

		return s_Instance;
	}

	bool WindowsInputManager::TranslateWindowsKeyCode(uint* keyCodePtr)
	{
		uint& keyCode = *keyCodePtr;
		*keyCodePtr = s_InputKeyCodeLookup[keyCode];
		return (bool)(*keyCodePtr);
	}
}
