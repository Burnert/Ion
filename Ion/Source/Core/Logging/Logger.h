#pragma once

namespace Ion
{
#pragma region Logger

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

	private:
		Logger(const String& name);

	private:
		String m_Name;
		TShared<spdlog::logger> m_Logger;
		ELogLevel m_LogLevel;
		bool m_bEnabled;

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
		if (m_bEnabled)
		{
			m_Logger->log((spdlog::level::level_enum)logLevel, str, Forward<Args>(args)...);
		}
#endif
	}

	inline void Logger::SetState(bool bEnabled)
	{
		m_bEnabled = true;
	}

	inline bool Logger::GetState() const
	{
		return m_bEnabled;
	}

#pragma endregion

#pragma region LogManager

#define REGISTER_LOGGER(varName, fullname) inline Logger& varName = LogManager::RegisterLogger(fullname)

	class ION_API LogManager
	{
	public:
		static Logger& RegisterLogger(const String& name);

		static Logger& GetLogger(const String& name);

		static void SetGlobalLogLevel(ELogLevel logLevel);
		static ELogLevel GetGlobalLogLevel();

	private:
		static bool IsLoggerNameValid(const String& name);

	private:
		THashMap<String, Logger> m_Loggers;

		static LogManager* s_Instance;
		static LogManager& Get();
	};

	inline LogManager& LogManager::Get()
	{
		if (!s_Instance)
			s_Instance = new LogManager;
		return *s_Instance;
	}

#pragma endregion

	// Global / Generic Engine Logger
	REGISTER_LOGGER(GEngineLogger, "Engine");
	// Core Logger
	REGISTER_LOGGER(CoreLogger, "Core");
}
