#include "EditorPCH.h"

#include "LogSettings.h"

#include "UserInterface/ImGui.h"

namespace Ion::Editor
{
#pragma region UI

	UILogSettings::UILogSettings(LogSettings* owner) :
		m_Owner(owner),
		bWindowOpen(false)
	{
	}

	void UILogSettings::Draw()
	{
		if (bWindowOpen)
		{
			if (ImGui::Begin("Logging Settings", &bWindowOpen))
			{
				const LogManager::HierarchyNode& loggerHierarchyRoot = LogManager::GetLoggerHierarchy();

				static ImGuiTableFlags flags =
					ImGuiTableFlags_BordersV  |
					ImGuiTableFlags_BordersH  |
					ImGuiTableFlags_Resizable |
					ImGuiTableFlags_ScrollY   |
					ImGuiTableFlags_Sortable  |
					ImGuiTableFlags_NoBordersInBody;
				if (ImGui::BeginTable("table_ion_loggers", 4, flags))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Logger",    ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_DefaultSort);
					ImGui::TableSetupColumn("Log Level", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 100.0f);
					ImGui::TableSetupColumn("Enable",    ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, ImGui::CalcTextSize("Enable").x);
					ImGui::TableSetupColumn("Solo",      ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, ImGui::CalcTextSize("Solo").x);
					ImGui::TableHeadersRow();

					ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize());

					DrawLoggerNodeChildren(loggerHierarchyRoot, ImGui::TableGetSortSpecs());

					ImGui::PopStyleVar();

					ImGui::EndTable();
				}
			}
			ImGui::End();
		}
	}

	void UILogSettings::DrawLoggerNodeChildren(const LogManager::HierarchyNode& node, ImGuiTableSortSpecs* sortSpecs)
	{
		if (!node.HasChildren())
			return;

		// @TODO: Sorting should preferably not happen every frame.

		TArray<LogManager::HierarchyNode*> children = node.GetChildren();
		if (sortSpecs && sortSpecs->SpecsCount > 0)
		{
			std::sort(children.begin(), children.end(), [sortSpecs](LogManager::HierarchyNode* lhs, LogManager::HierarchyNode* rhs) -> bool
			{
				const ImGuiTableColumnSortSpecs* spec = &sortSpecs->Specs[0];
				int32 delta = lhs->Get().Name.compare(rhs->Get().Name);
				if (delta < 0)
					return (spec->SortDirection == ImGuiSortDirection_Ascending) ? 1 : 0;
				else
					return (spec->SortDirection == ImGuiSortDirection_Ascending) ? 0 : 1;
			});
		}

		for (const LogManager::HierarchyNode* child : children)
		{
			ionassert(child);
			DrawLoggerRow(*child, sortSpecs);
		}
	}

	void UILogSettings::DrawLoggerRow(const LogManager::HierarchyNode& node, ImGuiTableSortSpecs* sortSpecs)
	{
		const LoggerHierarchyEntry& entry = node.Get();
		bool bAlwaysActive = entry.Logger && entry.Logger->IsAlwaysActive();
		String nodeName = 
			bAlwaysActive ?
			entry.Name + "!" :
			entry.Name;

		ImGui::TableNextRow();

		// Logger
		ImGui::TableNextColumn();

		// Change the color for always active loggers
		if (bAlwaysActive)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));

		ImGuiTreeNodeFlags nodeFlags =
			ImGuiTreeNodeFlags_NoTreePushOnOpen |
			ImGuiTreeNodeFlags_SpanFullWidth    |
			FlagsIf(!node.HasChildren(), ImGuiTreeNodeFlags_Leaf);

		bool bNodeOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags) && node.HasChildren();
		
		if (bAlwaysActive)
			ImGui::PopStyleColor();

		// Push ID for the controls
		ImGui::PushID(nodeName.c_str());
		{
			if (ImGui::BeginPopupContextItem("logger_context"))
			{
				if (node.HasChildren())
				{
					ImGui::Text("Group Settings");

					ImGui::SetNextItemWidth(-FLT_MIN);
					if (ImGui::BeginCombo("Set Log Level", nullptr, ImGuiComboFlags_NoPreview))
					{
						for (int32 i = 0; i < 6; ++i)
						{
							if (ImGui::Selectable(TEnumParser<ELogLevel>::ToString((ELogLevel)i).c_str()))
							{
								LogManager::SetGroupLogLevel(entry.GroupName, (ELogLevel)i);
							}
						}

						ImGui::EndCombo();
					}

					if (ImGui::Button("Enable"))
					{
						LogManager::EnableGroup(entry.GroupName);
					}
					ImGui::SameLine();
					if (ImGui::Button("Disable"))
					{
						LogManager::DisableGroup(entry.GroupName);
					}

					if (ImGui::Button("Solo"))
					{
						LogManager::EnableGroupSolo(entry.GroupName);
					}
				}
				else
				{
					ImGui::TextDisabled("No group settings.");
				}

				ImGui::EndPopup();
			}

			// Make the checkboxes smaller
			ImGuiStyle& style = ImGui::GetStyle();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));

			// Don't show controls if a logger does not exist in this node.
			if (entry.Logger)
			{
				// Log level
				ImGui::TableNextColumn();
				ELogLevel logLevel = entry.Logger->GetLevel();
				String sLogLevel = TEnumParser<ELogLevel>::ToString(logLevel);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::BeginCombo("##log_level", sLogLevel.c_str(), ImGuiComboFlags_None))
				{
					for (int32 i = 0; i < 6; ++i)
					{
						if (ImGui::Selectable(TEnumParser<ELogLevel>::ToString((ELogLevel)i).c_str(), logLevel == (ELogLevel)i))
						{
							entry.Logger->SetLevel((ELogLevel)i);
						}
					}

					ImGui::EndCombo();
				}

				bool bSoloMode = LogManager::IsSoloModeEnabled();

				// Enable
				ImGui::TableNextColumn();
				// Render the checkbox as disabled if solo mode is enabled.
				if (bSoloMode)
					ImGui::PushDisabledStyle();
				bool bEnabled = entry.Logger->GetState();
				if (ImGui::Checkbox("##enabled", &bEnabled) && !bSoloMode)
				{
					entry.Logger->SetState(bEnabled);
				}
				if (bSoloMode)
					ImGui::PopDisabledStyle();

				// Solo
				ImGui::TableNextColumn();
				bool bSolo = entry.Logger->IsSoloed();
				if (ImGui::Checkbox("##solo", &bSolo))
				{
					if (bSolo)
						entry.Logger->Solo();
					else
						entry.Logger->Unsolo();
				}
			}

			ImGui::PopStyleVar(2);
		}
		ImGui::PopID();

		if (bNodeOpen)
		{
			ImGui::TreePush(nodeName.c_str());
			DrawLoggerNodeChildren(node, sortSpecs);
			ImGui::TreePop();
		}
	}

#pragma endregion

#pragma region Behaviour

	LogSettings::LogSettings() :
		m_UI(new UILogSettings(this))
	{
	}

	LogSettings::~LogSettings()
	{
		ionassert(m_UI);
		delete m_UI;
	}

#pragma endregion
}
