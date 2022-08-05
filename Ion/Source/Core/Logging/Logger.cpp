#include "IonPCH.h"

#include "Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Ion
{
#ifdef ION_ENGINE
	TShared<spdlog::logger> Logger::s_EngineLogger;
#endif

	TShared<spdlog::logger> Logger::s_ClientLogger;

	void Logger::Init()
	{
		Platform::SetConsoleOutputUTF8();

		spdlog::set_pattern("%^[%n] %T : %v%$");

#ifdef ION_ENGINE
		s_EngineLogger = spdlog::stdout_color_mt("Engine");
		s_EngineLogger->set_level(spdlog::level::trace);
#endif

		s_ClientLogger = spdlog::stdout_color_mt("Client");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}
