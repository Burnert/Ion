#include "IonPCH.h"

#include "LogManager.h"

namespace Ion
{
	Logger& LogManager::RegisterLogger(const String& name, uint8 loggerFlags)
	{
		LogManager& instance = Get();

		ionassert(IsLoggerNameValid(name), "Logger name \"{}\" is invalid.", name);
		ionassert(instance.m_Loggers.find(name) == instance.m_Loggers.end(), "A logger with name \"{}\" already exists.", name);

		LoggerMapEntry& entry = instance.m_Loggers.emplace(name, LoggerMapEntry { Logger(name, loggerFlags), nullptr }).first->second;

		entry.HierarchyNode = &AddHierarchyNode(entry.Logger);

		return entry.Logger;
	}

	Logger& LogManager::GetLogger(const String& name)
	{
		LogManager& instance = Get();

		ionassert(instance.m_Loggers.find(name) != instance.m_Loggers.end(), "Cannot find a logger with name \"{}\".", name);
		return instance.m_Loggers.at(name).Logger;
	}

	void LogManager::SetGlobalLogLevel(ELogLevel logLevel)
	{
		spdlog::set_level((spdlog::level::level_enum)logLevel);
	}

	ELogLevel LogManager::GetGlobalLogLevel()
	{
		return (ELogLevel)spdlog::get_level();
	}

	const LogManager::HierarchyNode& LogManager::GetLoggerHierarchy()
	{
		return *Get().m_LoggerHierarchy;
	}

	void LogManager::EnableSolo(const Logger& logger)
	{
		LogManager& instance = Get();

		instance.m_SoloedLoggers.insert(logger.m_Name);
	}

	void LogManager::DisableSolo(const Logger& logger)
	{
		LogManager& instance = Get();

		instance.m_SoloedLoggers.erase(logger.m_Name);
	}

	bool LogManager::IsSoloed(const Logger& logger)
	{
		LogManager& instance = Get();

		return instance.m_SoloedLoggers.find(logger.m_Name) != instance.m_SoloedLoggers.end();
	}

	void LogManager::UnsoloAll()
	{
		LogManager& instance = Get();

		instance.m_SoloedLoggers.clear();
	}

	bool LogManager::IsSoloModeEnabled()
	{
		LogManager& instance = Get();

		return !instance.m_SoloedLoggers.empty();
	}

	bool LogManager::IsLoggerNameValid(const String& name)
	{
		// Make sure there aren't any invalid characters in the name first
		// Valid characters: a-z A-Z 0-9 -_:
		if (std::find_if(name.begin(), name.end(), [](char c)
		{
			return !(c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9' || IsAnyOf(c, "-_:"));
		}) != name.end())
			return false;

		TArray<String> splitName = SplitString(name, "::"s);
		// Find invalid segments
		// There can't be empty segments and segments with ":" characters.
		if (std::find_if(splitName.begin(), splitName.end(), [](const String& str)
		{
			return str.empty() || std::find(str.begin(), str.end(), ':') != str.end();
		}) != splitName.end())
			return false;

		return true;
	}

	LogManager::HierarchyNode& LogManager::AddHierarchyNode(Logger& logger)
	{
		TArray<String> splitName = SplitString(logger.m_Name, "::"s);

		HierarchyNode* currentNode = Get().m_LoggerHierarchy.get();
		for (const String& segment : splitName)
		{
			auto& children = currentNode->GetChildren();
			auto it = std::find_if(children.begin(), children.end(), [&segment](HierarchyNode* const& node) { return node->Get().Name == segment; });
			if (it == children.end())
			{
				currentNode = &currentNode->Insert(HierarchyNode::Make({ segment, nullptr }));
			}
			else
			{
				currentNode = *it;
			}
		}
		currentNode->Get().Logger = &logger;
		return *currentNode;
	}

	LogManager::LogManager()
	{
		ionassert(!s_Instance);
		m_LoggerHierarchy.reset(&HierarchyNode::Make({ ""s, nullptr }));
	}

	LogManager* LogManager::s_Instance = nullptr;
}
