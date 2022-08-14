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
		Logger* Logger;
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

		static Logger& RegisterLogger(const String& name);

		static Logger& GetLogger(const String& name);

		static void SetGlobalLogLevel(ELogLevel logLevel);
		static ELogLevel GetGlobalLogLevel();

		static const HierarchyNode& GetLoggerHierarchy();

	private:
		static bool IsLoggerNameValid(const String& name);

		static HierarchyNode& AddHierarchyNode(Logger& logger);

		LogManager();

	private:
		THashMap<String, LoggerMapEntry> m_Loggers;
		TUnique<HierarchyNode> m_LoggerHierarchy;

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
