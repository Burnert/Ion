#include "IonPCH.h"

#include "Logger.h"
#include "LogManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ion
{
	Logger& Logger::Register(const String& name)
	{
		return LogManager::RegisterLogger(name);
	}

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
}
