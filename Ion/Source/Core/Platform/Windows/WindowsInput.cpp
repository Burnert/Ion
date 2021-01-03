#include "IonPCH.h"

#include "WindowsInput.h"

namespace Ion
{
    bool WindowsInputManager::IsKeyPressed(KeyCode keyCode) const
    {
        return false;
    }

    bool WindowsInputManager::IsMouseButtonPressed(MouseButton mouseButton) const
    {
        return false;
    }

    bool WindowsInputManager::TranslateKeyCode(uint* keyCodePtr)
    {
        uint& keyCode = *keyCodePtr;
        *keyCodePtr = s_InputKeyCodeLookup[keyCode];
        return (bool)(*keyCodePtr);
    }
}
