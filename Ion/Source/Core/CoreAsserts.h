#pragma once

#include "CoreMacros.h"
#include "Logging/Logger.h"

#if ION_PLATFORM_WINDOWS
#define MBRESULTCANCEL 2 // IDCANCEL
#else
#define MBRESULTCANCEL -1
#endif

namespace Ion
{
	class AssertionHelper // No DLL export
	{
		// @TODO: Add call stack logging
	public:
		static constexpr const char* AssertFailedLog = "Assertion failed: {0}\n{4}  function: {1}\n  in {2}:{3}";
		static constexpr const char* AssertFailedExLog = "Assertion exception: {0}\n{4}  function: {1}\n  in {2}:{3}";
		static constexpr const char* AssertFailedPrintf = "Assertion failed: %s\n%s  function: %s\n  in %s:%d";

		inline static int ShowMessageBox(const char* expression, const char* function, const char* file, int line, const char* message = "")
		{
#if ION_PLATFORM_WINDOWS
			char buffer[550];
			sprintf_s(buffer, AssertFailedPrintf, expression, message, function, file, line);
			wchar bufferW[550];
			MultiByteToWideChar(CP_UTF8, 0, buffer, -1, bufferW, 550);
			return MessageBox(nullptr, bufferW, TEXT("Ion Assertion Failure!"), MB_RETRYCANCEL | MB_ICONERROR);
#else
			return MBRESULTCANCEL;
#endif
		}

		template<typename... Args>
		inline static int HandleFail(const char* expression, const char* function, const char* file, int line, const char* format = nullptr, Args&&... args)
		{
			if (format != nullptr)
			{
				char message[500] = "  ";
				sprintf_s(message + 2, 497, format, args...);
				strcat_s(message, "\n");
				LOG_CRITICAL(AssertFailedLog, expression, function, file, line, message);
#if !ION_LOG_ENABLED || ION_ASSERTS_FORCE_MSGBOX
				if (ShowMessageBox(expression, function, file, line, message) == MBRESULTCANCEL) abort();
#endif
			}
			else
			{
				LOG_CRITICAL(AssertFailedLog, expression, function, file, line, "");
#if !ION_LOG_ENABLED || ION_ASSERTS_FORCE_MSGBOX
				if (ShowMessageBox(expression, function, file, line) == MBRESULTCANCEL) abort();
#endif
			}
			return 0;
		}

		template<typename... Args>
		inline static int HandleFailEx(const char* expression, const char* function, const char* file, int line, const char* format = nullptr, Args&&... args)
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

#if !ION_DIST
// Stops the execution if the expression evaluates to false (terminates the program on distribution builds!)
#define ionassertnd(x, ...) \
(void)(!!(x) || Ion::AssertionHelper::HandleFail(#x, __FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__) || (debugbreak(), 0))
#else
// Stops the execution if the expression evaluates to false (terminates the program on distribution builds!)
#define ionassertnd(x, ...) \
(void)(!!(x) || Ion::AssertionHelper::HandleFail(#x, __FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__) || (debugbreak(), 0) || (abort(), 0))
#endif

/* Logs an assertion failure if the expression evaluates to false and executes a specified fallback code
  Example:
  ionexcept(value != 0)
  { return -1; }
In Debug: Stops the execution first */
#define ionexcept(x, ...) \
if (!!(x) || Ion::AssertionHelper::HandleFailEx(#x, __FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__) || (debugbreakd(), 0)); else

#if ION_DEBUG
// Stops the execution if the expression evaluates to false (debug code only!)
#define ionassert(x, ...) \
(void)(!!(x) || Ion::AssertionHelper::HandleFail(#x, __FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__) || (debugbreak(), 0))
#else
#define ionassert(x, ...)
#endif
