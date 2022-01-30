#include "IonPCH.h"

#include "ImGui.h"
#include "ImGuiInternal.h"

namespace ImGui
{
	Ion::Vector4 GetWindowWorkRect()
	{
		ImGuiWindow* window = GetCurrentWindow();
		Ion::Vector4 rect;
		rect.x = window->WorkRect.Min.x;
		rect.y = window->WorkRect.Min.y;
		rect.z = window->WorkRect.Max.x;
		rect.w = window->WorkRect.Max.y;
		return rect;
	}
}
