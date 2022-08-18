#pragma once

#include "Core/Logging/LogManager.h"

struct ImGuiTableSortSpecs;

namespace Ion::Editor
{
#pragma region UI

	class LogSettings;

	class EDITOR_API UILogSettings
	{
	public:
		UILogSettings(LogSettings* owner);

		void Draw();

	private:
		void DrawLoggerNodeChildren(const LogManager::HierarchyNode& node, ImGuiTableSortSpecs* sortSpecs);
		void DrawLoggerRow(const LogManager::HierarchyNode& node, ImGuiTableSortSpecs* sortSpecs);

	private:
		LogSettings* m_Owner;

	public:
		bool bWindowOpen;
	};

#pragma endregion

#pragma region Behaviour

	class EDITOR_API LogSettings
	{
	public:
		LogSettings();
		~LogSettings();

		void DrawUI();

		UILogSettings& GetUI();

	private:
		UILogSettings* m_UI;
	};

	inline void LogSettings::DrawUI()
	{
		ionassert(m_UI);
		m_UI->Draw();
	}

	inline UILogSettings& LogSettings::GetUI()
	{
		ionassert(m_UI);
		return *m_UI;
	}

#pragma endregion
}
