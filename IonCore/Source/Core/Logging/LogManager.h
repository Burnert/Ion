#pragma once

/**
 * This file is not included in the Core.h because it's only needed in certain situations.
 * Logger.h, on the other hand, is used everywhere.
 */

#include "Core/Container/Tree.h"

namespace Ion
{
	struct LoggerHierarchyEntry
	{
		String Name;
		String GroupName;
		Logger* Logger;
		bool bDebugOnly;
	};

	struct LoggerMapEntry
	{
		Logger Logger;
		TTreeNode<LoggerHierarchyEntry>* HierarchyNode;
	};

	class ION_API LogManager
	{
	public:
		using HierarchyNode = TTreeNode<LoggerHierarchyEntry>;

		static Logger& RegisterLogger(const String& name, uint8 loggerFlags, ELogLevel defaultLogLevel, bool bDebugOnly);
		static Logger_NoImpl RegisterNoImplDebugLogger(const String& name, uint8 loggerFlags, ELogLevel defaultLogLevel);

		static Logger& GetLogger(const String& name);

		static void SetGlobalLogLevel(ELogLevel logLevel);
		static ELogLevel GetGlobalLogLevel();

		static const HierarchyNode& GetLoggerHierarchy();

		static void EnableSolo(const Logger& logger);
		static void DisableSolo(const Logger& logger);
		static bool IsSoloed(const Logger& logger);

		static void UnsoloAll();
		static bool IsSoloModeEnabled();

		// Group functions

		static void SetGroupLogLevel(const String& groupName, ELogLevel logLevel);
		static void EnableGroup(const String& groupName);
		static void DisableGroup(const String& groupName);
		static void EnableGroupSolo(const String& groupName);

	private:
		static bool IsLoggerNameValid(const String& name);

		static HierarchyNode* AddHierarchyNodeHelper(const String& name);
		static HierarchyNode& AddHierarchyNode(Logger& logger);
		static HierarchyNode& AddUnavailableHierarchyNode(const String& name);

		static HierarchyNode* FindGroupNode(const String& groupName);

		LogManager();

	private:
		THashMap<String, LoggerMapEntry> m_Loggers;
		std::unique_ptr<HierarchyNode> m_LoggerHierarchy;
		THashSet<String> m_SoloedLoggers;

		static LogManager* s_Instance;
		static LogManager& Get();
	};

	inline LogManager& LogManager::Get()
	{
		if (!s_Instance)
			s_Instance = new LogManager;
		return *s_Instance;
	}
}
