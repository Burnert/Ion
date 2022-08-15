#include "IonPCH.h"

#include "Logger.h"
#include "LogManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ion
{
	Logger& Logger::Register(const String& name, bool bAlwaysActive)
	{
		return LogManager::RegisterLogger(name, bAlwaysActive);
	}

	Logger::Logger(const String& name, bool bAlwaysActive) :
		m_Name(name),
		m_LogLevel(ELogLevel::Trace),
		m_bEnabled(true),
		m_bAlwaysActive(bAlwaysActive),
		m_Logger(spdlog::stdout_color_mt(name))
	{
		if (m_bAlwaysActive)
			m_Logger->set_pattern("%^[%n]! %T : %v%$");
		else
			m_Logger->set_pattern("%^[%n] %T : %v%$");
		m_Logger->set_level(spdlog::level::trace);
	}

	bool Logger::ShouldLog() const
	{
		return m_bEnabled && (m_bAlwaysActive || !LogManager::IsSoloModeEnabled() || IsSoloed());
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

	void Logger::Solo()
	{
		LogManager::EnableSolo(*this);
	}

	void Logger::Unsolo()
	{
		LogManager::DisableSolo(*this);
	}

	bool Logger::IsSoloed() const
	{
		return LogManager::IsSoloed(*this);
	}

	void Logger::UnsoloAll()
	{
		LogManager::UnsoloAll();
	}
}
