#include "IonPCH.h"

#include "Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ion
{
	Logger::Logger(const String& name) :
		m_Name(name),
		m_LogLevel(ELogLevel::Trace),
		m_bEnabled(true),
		m_Logger(spdlog::stdout_color_mt(name))
	{
		m_Logger->set_pattern("%^[%n] %T : %v%$");
		m_Logger->set_level(spdlog::level::trace);
	}

	void Logger::SetLevel(ELogLevel logLevel)
	{
		m_LogLevel = logLevel;
		m_Logger->set_level((spdlog::level::level_enum)logLevel);
	}

	ELogLevel Logger::GetLevel() const
	{
		ionassert(m_LogLevel == (ELogLevel)m_Logger->level());
		return m_LogLevel;
	}

	Logger& LogManager::RegisterLogger(const String& name)
	{
		ionassert(IsLoggerNameValid(name), "Logger name \"{}\" is invalid.", name);
		ionassert(Get().m_Loggers.find(name) == Get().m_Loggers.end(), "A logger with name \"{}\" already exists.", name);
		return Get().m_Loggers.emplace(name, Logger(name)).first->second;
	}

	Logger& LogManager::GetLogger(const String& name)
	{
		ionassert(Get().m_Loggers.find(name) != Get().m_Loggers.end(), "Cannot find a logger with name \"{}\".", name);
		return Get().m_Loggers.at(name);
	}

	void LogManager::SetGlobalLogLevel(ELogLevel logLevel)
	{
		spdlog::set_level((spdlog::level::level_enum)logLevel);
	}

	ELogLevel LogManager::GetGlobalLogLevel()
	{
		return (ELogLevel)spdlog::get_level();
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

	LogManager* LogManager::s_Instance = nullptr;
}
