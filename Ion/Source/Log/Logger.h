#pragma once

#include "Core/CoreApi.h"
#include "spdlog/spdlog.h"

namespace Ion
{ 
	class ION_API Logger
	{
	public:
		static void Init();

		// Client Logger

		FORCEINLINE static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

#ifdef ION_ENGINE
		// Engine Logger

	public:
		FORCEINLINE static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_EngineLogger;
#endif
	};
}

#ifdef ION_LOG_ENABLED

#ifdef ION_ENGINE

#define ION_LOG_ENGINE_TRACE(...)      Ion::Logger::GetEngineLogger()->trace(__VA_ARGS__)
#define ION_LOG_ENGINE_DEBUG(...)      Ion::Logger::GetEngineLogger()->debug(__VA_ARGS__)
#define ION_LOG_ENGINE_INFO(...)       Ion::Logger::GetEngineLogger()->info(__VA_ARGS__)
#define ION_LOG_ENGINE_WARN(...)       Ion::Logger::GetEngineLogger()->warn(__VA_ARGS__)
#define ION_LOG_ENGINE_ERROR(...)      Ion::Logger::GetEngineLogger()->error(__VA_ARGS__)
#define ION_LOG_ENGINE_CRITICAL(...)   Ion::Logger::GetEngineLogger()->critical(__VA_ARGS__)

#define ION_LOG_ENGINE_BADPLATFORMFUNCTIONCALL() ION_LOG_ENGINE_CRITICAL("{0} is not supposed to be called on this platform!", __FUNCTION__)

#endif

#define ION_LOG_TRACE(...)             Ion::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define ION_LOG_DEBUG(...)             Ion::Logger::GetClientLogger()->debug(__VA_ARGS__)
#define ION_LOG_INFO(...)              Ion::Logger::GetClientLogger()->info(__VA_ARGS__)
#define ION_LOG_WARN(...)              Ion::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define ION_LOG_ERROR(...)             Ion::Logger::GetClientLogger()->error(__VA_ARGS__)
#define ION_LOG_CRITICAL(...)          Ion::Logger::GetClientLogger()->critical(__VA_ARGS__)

#else

#ifdef ION_ENGINE

#define ION_LOG_ENGINE_TRACE(...)
#define ION_LOG_ENGINE_DEBUG(...)
#define ION_LOG_ENGINE_INFO(...)
#define ION_LOG_ENGINE_WARN(...)
#define ION_LOG_ENGINE_ERROR(...)
#define ION_LOG_ENGINE_CRITICAL(...)

#define ION_LOG_ENGINE_BADPLATFORMFUNCTIONCALL(...)

#endif

#define ION_LOG_TRACE(...)
#define ION_LOG_DEBUG(...)
#define ION_LOG_INFO(...)
#define ION_LOG_WARN(...)
#define ION_LOG_ERROR(...)
#define ION_LOG_CRITICAL(...)

#endif