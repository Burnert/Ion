#include "IonPCH.h"

#include "WindowsInput.h"

namespace Ion
{
	std::shared_ptr<InputManager> InputManager::Get()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = std::make_shared<WindowsInputManager>();
		}
		return s_Instance;
	}

	bool WindowsInputManager::TranslateWindowsKeyCode(uint* keyCodePtr)
	{
		uint& keyCode = *keyCodePtr;
		*keyCodePtr = s_InputKeyCodeLookup[keyCode];
		return (bool)(*keyCodePtr);
	}
}
