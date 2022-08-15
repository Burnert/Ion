#pragma once

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define SPDLOG_COMPILED_LIB
#include "spdlog/spdlog.h"

namespace Ion
{
#define REGISTER_LOGGER(varName, fullname) inline Logger& varName = Logger::Register(fullname)

/**
 * Create a logger that is active even if logger solo mode is enabled.
 */
#define REGISTER_LOGGER_ALWAYS_ACTIVE(varName, fullname) inline Logger& varName = Logger::Register(fullname, true)

	enum class ELogLevel : uint8
	{
		Trace,
		Debug,
		Info,
		Warn,
		Error,
		Critical,
	};

	class ION_API Logger
	{
	public:
		static Logger& Register(const String& name, bool bAlwaysActive = false);

		template<typename TStr, typename... Args>
		void Trace(const TStr& str, Args&&... args) const;
		template<typename TStr, typename... Args>
		void Debug(const TStr& str, Args&&... args) const;
		template<typename TStr, typename... Args>
		void Info(const TStr& str, Args&&... args) const;
		template<typename TStr, typename... Args>
		void Warn(const TStr& str, Args&&... args) const;
		template<typename TStr, typename... Args>
		void Error(const TStr& str, Args&&... args) const;
		template<typename TStr, typename... Args>
		void Critical(const TStr& str, Args&&... args) const;

		template<typename TStr, typename... Args>
		void Log(ELogLevel logLevel, const TStr& str, Args&&... args) const;

		void SetLevel(ELogLevel logLevel);
		ELogLevel GetLevel() const;

		void SetState(bool bEnabled);
		bool GetState() const;

		void Solo();
		void Unsolo();
		bool IsSoloed() const;

		bool IsAlwaysActive() const;

		const String& GetName() const;

		static void UnsoloAll();

	private:
		Logger(const String& name, bool bAlwaysActive);

		bool ShouldLog() const;

	private:
		String m_Name;
		TShared<spdlog::logger> m_Logger;
		ELogLevel m_LogLevel;
		bool m_bEnabled;
		bool m_bAlwaysActive;

		friend class LogManager;
	};

	template<typename TStr, typename... Args>
	inline void Logger::Trace(const TStr& str, Args&&... args) const
	{
		Log(ELogLevel::Trace, str, Forward<Args>(args)...);
	}

	template<typename TStr, typename... Args>
	inline void Logger::Debug(const TStr& str, Args&&... args) const
	{
		Log(ELogLevel::Debug, str, Forward<Args>(args)...);
	}

	template<typename TStr, typename... Args>
	inline void Logger::Info(const TStr& str, Args&&... args) const
	{
		Log(ELogLevel::Info, str, Forward<Args>(args)...);
	}

	template<typename TStr, typename... Args>
	inline void Logger::Warn(const TStr& str, Args&&... args) const
	{
		Log(ELogLevel::Warn, str, Forward<Args>(args)...);
	}

	template<typename TStr, typename... Args>
	inline void Logger::Error(const TStr& str, Args&&... args) const
	{
		Log(ELogLevel::Error, str, Forward<Args>(args)...);
	}

	template<typename TStr, typename... Args>
	inline void Logger::Critical(const TStr& str, Args&&... args) const
	{
		Log(ELogLevel::Critical, str, Forward<Args>(args)...);
	}

	template<typename TStr, typename... Args>
	inline void Logger::Log(ELogLevel logLevel, const TStr& str, Args&&... args) const
	{
#if !ION_DIST
		if (ShouldLog())
		{
			m_Logger->log((spdlog::level::level_enum)logLevel, str, Forward<Args>(args)...);
		}
#endif
	}

	inline void Logger::SetState(bool bEnabled)
	{
		m_bEnabled = bEnabled;
	}

	inline bool Logger::GetState() const
	{
		return m_bEnabled;
	}

	inline bool Logger::IsAlwaysActive() const
	{
		return m_bAlwaysActive;
	}

	inline const String& Logger::GetName() const
	{
		return m_Name;
	}

	// Global / Generic Engine Logger
	REGISTER_LOGGER(GEngineLogger, "Engine");
	// Core Logger
	REGISTER_LOGGER_ALWAYS_ACTIVE(CoreLogger, "Core");
}
