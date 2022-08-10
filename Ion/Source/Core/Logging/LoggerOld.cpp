#include "IonPCH.h"

#include "LoggerOld.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ion
{
#ifdef ION_ENGINE
	TShared<spdlog::logger> LoggerOld::s_EngineLogger;
#endif

	TShared<spdlog::logger> LoggerOld::s_ClientLogger;

	void LoggerOld::Init()
	{
		Platform::SetConsoleOutputUTF8();

		spdlog::set_pattern("%^[%n] %T : %v%$");

#ifdef ION_ENGINE
		s_EngineLogger = spdlog::stdout_color_mt("Engine_OLD");
		s_EngineLogger->set_level(spdlog::level::trace);
#endif

		s_ClientLogger = spdlog::stdout_color_mt("Client_OLD");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}
