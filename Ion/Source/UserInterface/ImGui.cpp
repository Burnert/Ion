#include "IonPCH.h"

#include "ImGui.h"
#include "ImGuiInternal.h"

namespace ImGui
{
	bool IsTreeNodeToggled()
	{
		return ImGui::GetItemStatusFlags() & ImGuiItemStatusFlags_ToggledOpen;
	}

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

	// Widgets

	IMGUI_API bool CollapsingHeaderUnframed(const char* label, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		flags |=
			ImGuiTreeNodeFlags_NoTreePushOnOpen |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding |
			ImGuiTreeNodeFlags_NoAutoOpenOnLog;
		return TreeNodeBehavior(window->GetID(label), flags, label);
	}

	IMGUI_API bool CollapsingHeaderUnframed(const char* label, bool* p_visible, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		if (p_visible && !*p_visible)
			return false;

		ImGuiID id = window->GetID(label);
		flags |=
			ImGuiTreeNodeFlags_NoTreePushOnOpen |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding |
			ImGuiTreeNodeFlags_NoAutoOpenOnLog;
		if (p_visible)
			flags |= ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
		bool is_open = TreeNodeBehavior(id, flags, label);
		if (p_visible != NULL)
		{
			// Create a small overlapping close button
			ImGuiContext& g = *GImGui;
			ImGuiLastItemData last_item_backup;
			float button_size = g.FontSize;
			float button_x = ImMax(g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.x - g.Style.FramePadding.x * 2.0f - button_size);
			float button_y = g.LastItemData.Rect.Min.y;
			ImGuiID close_button_id = GetIDWithSeed("#CLOSE", NULL, id);
			if (CloseButton(close_button_id, ImVec2(button_x, button_y)))
				*p_visible = false;
			g.LastItemData = last_item_backup;
		}

		return is_open;
	}

	IMGUI_API void PushDisabledStyle()
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.25f, 0.25f, 0.28f, 0.25f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.28f, 0.25f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.25f, 0.25f, 0.28f, 0.25f));
		ImGui::PushStyleColor(ImGuiCol_CheckMark,      ImVec4(0.25f, 0.25f, 0.28f, 0.5f));
	}

	IMGUI_API void PopDisabledStyle()
	{
		ImGui::PopStyleColor(4);
	}
}
