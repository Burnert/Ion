#pragma once

#include "CoreMacros.h"
#include "Logging/Logger.h"
#include "Core/StringConverter.h"

#if ION_PLATFORM_WINDOWS
#define MBRESULTCANCEL 2 // IDCANCEL
#else
#define MBRESULTCANCEL -1
#endif

#if !ION_FORCE_NO_ASSERTS

namespace Ion
{
	class AssertionHelper // No DLL export (inline class)
	{
		// @TODO: Add call stack logging
	public:
		static constexpr const char* AssertFailedLog = "Assertion failed: {0}\n{4}  function: {1}\n  in {2}:{3}";
		static constexpr const char* AssertFailedExLog = "Assertion exception: {0}\n{4}  function: {1}\n  in {2}:{3}";
		static constexpr const char* AssertFailedPrintf = "Assertion failed: %s\n%s  function: %s\n  in %s:%d";

		inline static int32 ShowMessageBox(const char* expression, const char* function, const char* file, int32 line, const char* message = "")
		{
#if ION_PLATFORM_WINDOWS
			char buffer[550];
			sprintf_s(buffer, AssertFailedPrintf, expression, message, function, file, line);
			wchar bufferW[550] { };
			StringConverter::CharToWChar(buffer, bufferW);
			return MessageBox(nullptr, bufferW, TEXT("Ion Assertion Failure!"), MB_RETRYCANCEL | MB_ICONERROR);
#else
			return MBRESULTCANCEL;
#endif
		}

		template<typename... Args>
		inline static int32 HandleFail(const char* expression, const char* function, const char* file, int32 line, const char* format = nullptr, Args&&... args)
		{
			if (format != nullptr)
			{
				char message[500] = "  ";
				sprintf_s(message + 2, 497, format, args...);
				strcat_s(message, "\n");
				LOG_CRITICAL(AssertFailedLog, expression, function, file, line, message);
#if !ION_LOG_ENABLED || ION_FORCE_ASSERT_MSGBOX
				if (ShowMessageBox(expression, function, file, line, message) == MBRESULTCANCEL) abort();
#endif
			}
			else
			{
				LOG_CRITICAL(AssertFailedLog, expression, function, file, line, "");
#if !ION_LOG_ENABLED || ION_FORCE_ASSERT_MSGBOX
				if (ShowMessageBox(expression, function, file, line) == MBRESULTCANCEL) abort();
#endif
			}
			return 0;
		}

		template<typename... Args>
		inline static int32 HandleFailEx(const char* expression, const char* function, const char* file, int32 line, const char* format = nullptr, Args&&... args)
		{
			if (format != nullptr)
			{
				char message[500] = "  ";
				sprintf_s(message + 2, 497, format, args...);
				strcat_s(message, "\n");
				LOG_WARN(AssertFailedExLog, expression, function, file, line, message);
			}
			else
			{
				LOG_WARN(AssertFailedExLog, expression, function, file, line, "");
			}
			return 0;
		}
	};
}

#if ION_BREAK_ON_EXCEPT
#define _exceptbreak() debugbreakd()
#else
#define _exceptbreak() ((void)0)
#endif
/* Logs an assertion failure if the expression evaluates to false and executes a specified fallback code
 * Example:
 * ionexcept(value != 0)
 * { return -1; }
 * In Debug: Stops the execution first 
 * Never deletes the expression in non-debug builds. */
#define ionexcept(x, ...) \
if (!!(x) || Ion::AssertionHelper::HandleFailEx(#x, __FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__) || (_exceptbreak(), 0)); else

/* A special type of ionexcept that returns 0 on failure. */
#define _ionexcept_r(x, ...) ionexcept(x, __VA_ARGS__) return 0

/* Stops the execution if the expression evaluates to false (terminates the program in distribution builds!)
 * Never deletes the expression in non-debug builds. */
#define ionassertnd(x, ...) \
(void)(!!(x) || Ion::AssertionHelper::HandleFail(#x, __FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__) || (debugbreak(), 0) || (abort(), 0))

#if ION_DEBUG && ION_ENABLE_DEBUG_ASSERTS
/* Stops the execution if the expression evaluates to false (debug code only!)
 * Deletes the expression from non-debug builds. */
#define ionassert(x, ...) \
(void)(!!(x) || Ion::AssertionHelper::HandleFail(#x, __FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__) || (debugbreak(), 0))
#else
#define ionassert(x, ...) ((void)0)
#endif
#else

#define ionexcept(x, ...) if (!(x))
#define ionassertnd(x, ...) (void)(x)
#define ionassert(x, ...) ((void)0)

#endif
