#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"
#include "Core/Platform/Platform.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
// Error handler ------------------------------------------------------------------------------------------

#pragma region Error Handler

#define _ASSERT_ARGS const char* expr, const char* func, const char* file, int32 line
#define _FWD_ASSERT_ARGS expr, func, file, line
#define _PASS_ASSERT_ARGS(expr) expr, __FUNCSIG__, __FILE__, __LINE__

#define _ABORT_LOG_PATTERN        "({0}) => false\n\nFunction: {1}\nAt {2}:{3}\n"
#define _ABORT_LOG_PATTERN_NOEXPR "Function: {1}\nAt {2}:{3}\n"

#if ION_RELEASE || ION_DIST
#define _SHOW_ABORT_MESSAGE_BOX 1
#else
#define _SHOW_ABORT_MESSAGE_BOX 0
#endif

	struct ErrorHandler
	{
		template<typename... Args>
		static void AssertAbort(_ASSERT_ARGS, const char* format = nullptr, Args&&... args);

		template<typename TResult>
		static void UnwrapAbort(_ASSERT_ARGS, const TResult& result);

		template<typename TError>
		static void ErrorAbort(_ASSERT_ARGS, const TError& error);

		template<typename TError>
		static void PrintErrorThrow(_ASSERT_ARGS, const TError& error);

		template<typename... Args>
		static void Break(const char* format = nullptr, Args&&... args);

		ErrorHandler() = delete;

	private:
		template<bool bExpr = true>
		static void AbortMessageBox(_ASSERT_ARGS, const char* reason, const char* message);
	};

	// Error handler impl -----------------------------------------------------------

	template<typename... Args>
	inline void ErrorHandler::AssertAbort(_ASSERT_ARGS, const char* format, Args&&... args)
	{
		LOG_CRITICAL("Critical error has occured!");
		LOG_CRITICAL("Assertion failed:\n\n" _ABORT_LOG_PATTERN, _FWD_ASSERT_ARGS);
		if (format)
			LOG_CRITICAL(format, Forward<Args>(args)...);

#if _SHOW_ABORT_MESSAGE_BOX
		String message = format ? fmt::format(format, Forward<Args>(args)...) : EmptyString;
		AbortMessageBox(_FWD_ASSERT_ARGS, "Assertion failed", message.c_str());
#endif
	}

	template<typename TResult>
	inline void ErrorHandler::UnwrapAbort(_ASSERT_ARGS, const TResult& result)
	{
		LOG_CRITICAL("Critical error has occured!");
		LOG_CRITICAL("Unwrap failed: => Result<{4}>\n\n" _ABORT_LOG_PATTERN_NOEXPR, _FWD_ASSERT_ARGS, result.GetErrorClassName());
		String message = result.GetErrorMessage();
		if (!message.empty())
			LOG_CRITICAL(message);

#if _SHOW_ABORT_MESSAGE_BOX
		String reason = fmt::format("Unwrap failed: => Result<{}>", result.GetErrorClassName());
		AbortMessageBox<false>(_FWD_ASSERT_ARGS, reason.c_str(), message.c_str());
#endif
	}

	template<typename TError>
	inline void ErrorHandler::ErrorAbort(_ASSERT_ARGS, const TError& error)
	{
		LOG_CRITICAL("Critical error has occured!");
		LOG_CRITICAL("Error: => {4}\n\n" _ABORT_LOG_PATTERN_NOEXPR, _FWD_ASSERT_ARGS, TError::ClassName);
		if (!error.Message.empty())
			LOG_CRITICAL(error.Message);

#if _SHOW_ABORT_MESSAGE_BOX
		String reason = fmt::format("Error: => {}", TError::ClassName);
		AbortMessageBox<false>(_FWD_ASSERT_ARGS, reason.c_str(), message.c_str());
#endif
	}

	template<typename TError>
	inline void ErrorHandler::PrintErrorThrow(_ASSERT_ARGS, const TError& error)
	{
		LOG_ERROR("An error has been thrown.");
		LOG_ERROR("Error: => Result<{4}>\n\n" _ABORT_LOG_PATTERN_NOEXPR, _FWD_ASSERT_ARGS, TError::ClassName);
		if (!error.Message.empty())
			LOG_ERROR(error.Message);
	}

	template<typename... Args>
	inline void ErrorHandler::Break(const char* format, Args&&... args)
	{
		LOG_ERROR("A debugger break has been called.");
		if (format)
			LOG_ERROR(format, Forward<Args>(args)...);
	}

	template<bool bExpr>
	inline void ErrorHandler::AbortMessageBox(_ASSERT_ARGS, const char* reason, const char* message)
	{
#define _MESSAGE_BOX_FORMAT_STRING(pattern) \
		"Critical error has occured!\n" \
		"{4}\n\n" /* Reason */ \
		pattern   /* (Expr) Function X At File:Line */ \
		"\n{5}"   /* Message */

		constexpr const char* format = bExpr ?
			_MESSAGE_BOX_FORMAT_STRING(_ABORT_LOG_PATTERN) :
			_MESSAGE_BOX_FORMAT_STRING(_ABORT_LOG_PATTERN_NOEXPR);

		String content = fmt::format(format, _FWD_ASSERT_ARGS, reason, message);

		Platform::MessageBox(StringConverter::StringToWString(content), StringConverter::StringToWString(reason),
			Platform::MBT_RetryCancel, Platform::MBI_Error);
	}

#pragma endregion

// Error struct -------------------------------------------------------------------------------------------

#pragma region Error

#define _ERROR_TYPE_CTOR_HELPER(type) type(const String& message) : Error(message) { } type() : Error() { }
#define DEFINE_ERROR_TYPE(type) struct type : Error { static constexpr const char* ClassName = #type; _ERROR_TYPE_CTOR_HELPER(type) }

	struct Error
	{
		static constexpr const char* ClassName = "Error";

		const String Message;

		Error(const String& message) :
			Message(message)
		{
		}

		Error()
			: Message()
		{
		}
	};

// Core Error types --------------------------------------

	DEFINE_ERROR_TYPE(IOError);
	DEFINE_ERROR_TYPE(FileNotFoundError);

#pragma endregion

// Result struct ------------------------------------------------------------------------------------------

#pragma region Result

	namespace _Detail
	{
		template<typename TRet, typename... TErr>
		struct ResultBase
		{
			static_assert(!TIsBaseOfV<TRet, Error>, "Primary Result type cannot be derived from Error.");
			static_assert((TIsBaseOfV<Error, TErr> && ...), "All secondary Result types must be derived from Error.");

			using TVoid = std::monostate;
			static constexpr bool IsVoid = TIsSameV<TRet, TVoid>;

			template<typename T>
			ResultBase(const T& value) :
				m_Value(value)
			{
				static_assert(TIsSameV<T, TRet> || (TIsSameV<T, TErr> || ...),
					"Returned type has not been specified in the Result.");
			}

			~ResultBase() { }

			template<typename FExec>
			TRet Unwrap(_ASSERT_ARGS, FExec onFail) const;

			TRet ValueOr(const TRet& fallback) const;

			template<typename F>
			ResultBase& Ok(F lambda);
			template<typename E, typename F>
			ResultBase& Err(F lambda);

			bool IsOk() const;

			template<typename T>
			bool Is() const;

			template<typename E>
			E&& ForwardThrow();

			ResultBase(const ResultBase&) = delete;
			ResultBase(ResultBase&&) = delete;
			ResultBase& operator=(const ResultBase&) = delete;
			ResultBase& operator=(ResultBase&&) = delete;

			operator bool() const;

		private:
			String GetErrorClassName() const;
			String GetErrorMessage() const;

		protected:
			TVariant<TRet, TErr...> m_Value;

			friend struct ErrorHandler;
		};

		template<typename TRet, typename... TErr>
		template<typename FExec>
		inline TRet ResultBase<TRet, TErr...>::Unwrap(_ASSERT_ARGS, FExec onFail) const
		{
			static_assert(TIsConvertibleV<FExec, TFunction<void()>>);

			if (!std::holds_alternative<TRet>(m_Value))
			{
				ErrorHandler::UnwrapAbort(_FWD_ASSERT_ARGS, *this);
				onFail();
			}

			return std::get<TRet>(m_Value);
		}

// Helper macro that passes all the necessary arguments automatically.
#define Unwrap() Unwrap(_PASS_ASSERT_ARGS(nullptr), [] { debugbreak(); abort(); })

		template<typename TRet, typename... TErr>
		inline TRet ResultBase<TRet, TErr...>::ValueOr(const TRet& fallback) const
		{
			static_assert(!IsVoid, "Cannot get a void value.");

			if (!std::holds_alternative<TRet>(m_Value))
				return fallback;

			return std::get<TRet>(m_Value);
		}

		template<typename TRet, typename... TErr>
		template<typename F>
		inline ResultBase<TRet, TErr...>& ResultBase<TRet, TErr...>::Ok(F lambda)
		{
			static_assert(TIsConvertibleV<F, TFunction<void(TIf<IsVoid, void, const TRemoveConst<TRet>&>)>>);

			if (std::holds_alternative<TRet>(m_Value))
				if constexpr (!IsVoid)
					lambda(std::get<TRet>(m_Value));
				else
					lambda();

			return *this;
		}

		template<typename TRet, typename... TErr>
		template<typename E, typename F>
		inline ResultBase<TRet, TErr...>& ResultBase<TRet, TErr...>::Err(F lambda)
		{
			static_assert(TIsBaseOfV<Error, E>);
			static_assert(TIsConvertibleV<F, TFunction<void(const TRemoveConst<E>&)>>);

			if (std::holds_alternative<E>(m_Value))
				lambda(std::get<E>(m_Value));

			return *this;
		}

		template<typename TRet, typename... TErr>
		template<typename T>
		inline bool ResultBase<TRet, TErr...>::Is() const
		{
			static_assert(TIsSameV<T, TRet> || (TIsSameV<T, TErr> || ...),
				"The type has not been specified in the Result.");

			return std::holds_alternative<T>(m_Value);
		}

		template<typename TRet, typename... TErr>
		template<typename E>
		inline E&& ResultBase<TRet, TErr...>::ForwardThrow()
		{
			static_assert((TIsSameV<E, TErr> || ...), "Cannot forward throw of the Error type, because is doesn't exist in this Result.");

			// Cannot forward throw if the value is not the same as the specified Error type
			if (!std::holds_alternative<E>(m_Value))
				abort();

			return Move(std::get<E>(m_Value));
		}

		template<typename TRet, typename... TErr>
		inline bool ResultBase<TRet, TErr...>::IsOk() const
		{
			return std::holds_alternative<TRet>(m_Value);
		}

		template<typename TRet, typename... TErr>
		inline ResultBase<TRet, TErr...>::operator bool() const
		{
			return IsOk();
		}

		template<typename TRet, typename... TErr>
		inline String ResultBase<TRet, TErr...>::GetErrorClassName() const
		{
			return std::visit([](auto&& val) -> String
			{
				using T = std::decay_t<decltype(val)>;
				if constexpr (TIsBaseOfV<Error, T>)
					return T::ClassName;
				else
					return EmptyString;
			}, m_Value);
		}

		template<typename TRet, typename... TErr>
		inline String ResultBase<TRet, TErr...>::GetErrorMessage() const
		{
			return std::visit([](auto&& val) -> String
			{
				using T = std::decay_t<decltype(val)>;
				if constexpr (TIsBaseOfV<Error, T>)
					if (!val.Message.empty())
						return fmt::format("{0}: {1}", T::ClassName, val.Message);
				return EmptyString;
			}, m_Value);
		}
	}

	// Final Result structs ---------------------------------------------------

	template<typename TRet, typename... TErr>
	struct Result : _Detail::ResultBase<TRet, TErr...>
	{
		template<typename T>
		Result(const T& value) :
			ResultBase(value)
		{
		}
	};

	template<typename... TErr>
	struct Result<void, TErr...> : _Detail::ResultBase<std::monostate, TErr...>
	{
		Result() :
			ResultBase(std::monostate())
		{
		}

		template<typename T>
		Result(const T& value) :
			ResultBase(value)
		{
		}
	};

	using Void = std::monostate;

#pragma endregion

// Assertion macros ---------------------------------------------------------------------------------------

#pragma region Assertion / Errors

#define ionbreak(...) (void)((Ion::ErrorHandler::Break(__VA_ARGS__), 0) || (debugbreak(), 0))

#undef ionassert
#if ION_DEBUG
#define ionassert(x, ...) (void)(!!(x) || (Ion::ErrorHandler::AssertAbort(_PASS_ASSERT_ARGS(#x), __VA_ARGS__), 0) || (debugbreak(), 0))
#else
#define ionassert(x, ...) ((void)0)
#endif

#define ionverify(x, ...) (void)(!!(x) || (Ion::ErrorHandler::AssertAbort(_PASS_ASSERT_ARGS(#x), __VA_ARGS__), 0) || (debugbreak(), 0) || (abort(), 0))

// Throw macro -------------------------------------------------------------------------------------------------

	namespace _Detail
	{
		inline static String _FormatThrowMessage() { return ""; }
		template<typename... Args>
		inline static String _FormatThrowMessage(const String& format, Args&&... args) { return fmt::format(format, Forward<Args>(args)...); }
	}

#if ION_DEBUG
#define ionthrow(error, ...) return [&] { auto err = error(Ion::_Detail::_FormatThrowMessage(__VA_ARGS__)); Ion::ErrorHandler::PrintErrorThrow(_PASS_ASSERT_ARGS(0), err); return err; }()
#else
#define ionthrow(error, ...) return error(Ion::_Detail::_FormatThrowMessage(__VA_ARGS__))
#endif
#define ionthrowif(cond, error, ...) if ((cond)) ionthrow(error, __VA_ARGS__)

#define fwdthrow(result, error) { if (result.Is<error>()) return Move(result.ForwardThrow<error>()); }

// Error macro -------------------------------------------------------------------------------------------------

#define ionerror(error, ...) (void)((Ion::ErrorHandler::ErrorAbort(_PASS_ASSERT_ARGS(0), error(Ion::_Detail::_FormatThrowMessage(__VA_ARGS__))), 0) || (debugbreak(), 0) || (abort(), 0))

#pragma endregion

}
