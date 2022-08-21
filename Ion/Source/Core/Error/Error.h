#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"
#include "Core/Platform/Platform.h"

namespace Ion
{
	/**
	 * @brief This class is here, so Logger.h doesn't have to be included in this file
	 */
	class ION_API ErrorLoggerInterface
	{
		static void Error(const String& message);
		static void Critical(const String& message);

		friend struct ErrorHandler;
	};

// Error handler ------------------------------------------------------------------------------------------

#pragma region Error Handler

#define _ASSERT_ARGS const char* expr, const char* func, const char* file, int32 line
#define _FWD_ASSERT_ARGS expr, func, file, line
#define _PASS_ASSERT_ARGS(expr) expr, __FUNCSIG__, __FILE__, __LINE__

#define _ABORT_LOG_PATTERN        "({0}) => false\n\nFunction: {1}\nAt {2}:{3}\n"
#define _ABORT_LOG_PATTERN_NOEXPR "Function: {1}\nAt {2}:{3}\n"

#if ION_FORCE_ABORT_MSGBOX || ION_RELEASE || ION_DIST
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
		ErrorLoggerInterface::Critical("Critical error has occured!");
		ErrorLoggerInterface::Critical(fmt::format("Assertion failed:\n\n" _ABORT_LOG_PATTERN, _FWD_ASSERT_ARGS));
		if (format)
			ErrorLoggerInterface::Critical(fmt::format(format, Forward<Args>(args)...));

#if _SHOW_ABORT_MESSAGE_BOX
		String message = format ? fmt::format(format, Forward<Args>(args)...) : EmptyString;
		AbortMessageBox(_FWD_ASSERT_ARGS, "Assertion failed", message.c_str());
#endif
	}

	template<typename TResult>
	inline void ErrorHandler::UnwrapAbort(_ASSERT_ARGS, const TResult& result)
	{
		ErrorLoggerInterface::Critical("Critical error has occured!");
		ErrorLoggerInterface::Critical(fmt::format("Unwrap failed: => Result<{4}>\n\n" _ABORT_LOG_PATTERN_NOEXPR, _FWD_ASSERT_ARGS, result.GetErrorClassName()));
		String message = result.GetErrorMessage();
		if (!message.empty())
			ErrorLoggerInterface::Critical(message);

#if _SHOW_ABORT_MESSAGE_BOX
		String reason = fmt::format("Unwrap failed: => Result<{}>", result.GetErrorClassName());
		AbortMessageBox<false>(_FWD_ASSERT_ARGS, reason.c_str(), message.c_str());
#endif
	}

	template<typename TError>
	inline void ErrorHandler::ErrorAbort(_ASSERT_ARGS, const TError& error)
	{
		ErrorLoggerInterface::Critical("Critical error has occured!");
		ErrorLoggerInterface::Critical(fmt::format("Error: => {4}\n\n" _ABORT_LOG_PATTERN_NOEXPR, _FWD_ASSERT_ARGS, TError::ClassName));
		if (!error.Message.empty())
			ErrorLoggerInterface::Critical(error.Message);

#if _SHOW_ABORT_MESSAGE_BOX
		String reason = fmt::format("Error: => {}", TError::ClassName);
		AbortMessageBox<false>(_FWD_ASSERT_ARGS, reason.c_str(), message.c_str());
#endif
	}

	template<typename TError>
	inline void ErrorHandler::PrintErrorThrow(_ASSERT_ARGS, const TError& error)
	{
		ErrorLoggerInterface::Error("An error has been thrown.");
		ErrorLoggerInterface::Error(fmt::format("Error: => Result<{4}>\n\n" _ABORT_LOG_PATTERN_NOEXPR, _FWD_ASSERT_ARGS, TError::ClassName));
		if (!error.Message.empty())
			ErrorLoggerInterface::Error(error.Message);
	}

	template<typename... Args>
	inline void ErrorHandler::Break(const char* format, Args&&... args)
	{
		ErrorLoggerInterface::Error("A debugger break has been called.");
		if (format)
			ErrorLoggerInterface::Error(fmt::format(format, Forward<Args>(args)...));
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

		Error() :
			Message()
		{
		}
	};

// Core Error types --------------------------------------

	DEFINE_ERROR_TYPE(IOError);
	DEFINE_ERROR_TYPE(FileNotFoundError);
	DEFINE_ERROR_TYPE(BadArgumentError);
	DEFINE_ERROR_TYPE(StringConversionError);
	DEFINE_ERROR_TYPE(PlatformError);

#pragma endregion

// Result struct ------------------------------------------------------------------------------------------

#pragma region Result

	template<typename TRet, typename... TErr>
	struct Result;

	namespace _Detail
	{
		template<typename T>
		struct TIsResult
		{
			static constexpr bool Value = false;
		};

		template<typename T, typename... E>
		struct TIsResult<Result<T, E...>>
		{
			static constexpr bool Value = true;
		};

		template<typename TRet, typename... TErr>
		struct ResultBase
		{
			static_assert(!TIsBaseOfV<TRet, Error>, "Primary Result type cannot be derived from Error.");
			static_assert((TIsBaseOfV<Error, TErr> && ...), "All secondary Result types must be derived from Error.");

			using TVoid = std::monostate;
			static constexpr bool IsVoid = TIsSameV<TRet, TVoid>;

			/**
			 * @brief Construct a new Result Base object that holds the specified value
			 * 
			 * @tparam T Value type, it must be one of the Result template types
			 * @param value The value
			 */
			template<typename T, TEnableIfT<!TIsResult<T>::Value>* = 0>
			ResultBase(const T& value);

			/**
			 * @brief Forward throw constructor
			 * Use fwdthrowall / mfwdthrowall macros for this.
			 * 
			 * @param fwdThrow Result to copy the error from.
			 */
			template<typename T, typename... E>
			ResultBase(Result<T, E...>&& fwdThrow);

			~ResultBase() { }

			/**
			 * @brief Get the value of this Result.
			 * If the Result in an Error variant, aborts the program.
			 */
			template<typename FExec>
			TRet Unwrap(_ASSERT_ARGS, FExec onFail) const;

			/**
			 * @brief Get the value of this Result, or if it is an Error variant,
			 * get the fallback value instead.
			 * 
			 * @param fallback The fallback value
			 */
			TRet UnwrapOr(const TRet& fallback) const;

			/**
			 * @brief Execute lambda on Ok variant
			 * 
			 * @tparam F void(const TRet&>) or void()
			 * @param lambda Function to execute
			 * @return ResultBase& Reference to this result (chainable)
			 */
			template<typename F>
			ResultBase& Ok(F lambda);

			/**
			 * @brief Execute lambda on Err variant with the specified Error type
			 * 
			 * @tparam E Error type
			 * @tparam F void(const E&)
			 * @param lambda Function to execute
			 * @return ResultBase& Reference to this result (chainable)
			 */
			template<typename E, typename F>
			ResultBase& Err(F lambda);

			/**
			 * @brief Execute lambda on any Err variant
			 *
			 * @tparam F void(auto&)
			 * @param lambda Function to execute
			 * @return ResultBase& Reference to this result (chainable)
			 */
			template<typename F>
			ResultBase& Err(F lambda);

			/**
			 * @brief Is this Result an Ok variant
			 */
			bool IsOk() const;

			/**
			 * @brief Does this Result hold type T
			 * 
			 * @tparam T Type to check
			 */
			template<typename T>
			bool Is() const;

			/**
			 * @brief Forward throw this Result's Error
			 * Use the fwdthrow / mfwdthrow macros for this.
			 */
			template<typename E>
			E&& ForwardThrow();

			/**
			 * @see ResultBase::IsOk
			 */
			operator bool() const;

			String GetErrorClassName() const;
			String GetErrorMessage() const;

		private:
			template<typename TCheck, typename T, typename... E>
			bool FoldForwardThrow(Result<T, E...>& fwdThrow);

			template<typename TCheck, typename F>
			bool FoldErr(F lambda);

		protected:
			TVariant<TRet, TErr...> m_Value;

			friend struct ErrorHandler;
			template<typename T, typename... E>
			friend struct ResultBase;
		};

		template<typename TRet, typename... TErr>
		template<typename T, TEnableIfT<!TIsResult<T>::Value>*>
		ResultBase<TRet, TErr...>::ResultBase(const T& value) :
			m_Value(value)
		{
			static_assert(TIsSameV<T, TRet> || (TIsSameV<T, TErr> || ...),
				"Returned type has not been specified in the Result.");
		}

		template<typename TRet, typename... TErr>
		template<typename T, typename... E>
		ResultBase<TRet, TErr...>::ResultBase(Result<T, E...>&& fwdThrow)
		{
			static_assert((TIsAnyOfV<E, TErr...> && ...),
				"Cannot forward throw, because an Error type doesn't exist in this Result.");

			// Make sure there is actually an error.
			if /*unlikely*/ (fwdThrow) abort();

			(FoldForwardThrow<E>(fwdThrow) || ...);
		}

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
		inline TRet ResultBase<TRet, TErr...>::UnwrapOr(const TRet& fallback) const
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
		template<typename F>
		inline ResultBase<TRet, TErr...>& ResultBase<TRet, TErr...>::Err(F lambda)
		{
			if (!std::holds_alternative<TRet>(m_Value))
			{
				(FoldErr<TErr>(lambda) || ...);
			}

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

		template<typename TRet, typename... TErr>
		template<typename TCheck, typename T, typename... E>
		inline bool ResultBase<TRet, TErr...>::FoldForwardThrow(Result<T, E...>& fwdThrow)
		{
			static_assert(TIsAnyOfV<TCheck, E...>);

			if constexpr (TIsAnyOfV<TCheck, TErr...>)
			{
				if (std::holds_alternative<TCheck>(fwdThrow.m_Value))
				{
					m_Value.emplace<TCheck>(std::get<TCheck>(fwdThrow.m_Value));
					return true;
				}
			}
			return false;
		}

		template<typename TRet, typename ...TErr>
		template<typename TCheck, typename F>
		inline bool ResultBase<TRet, TErr...>::FoldErr(F lambda)
		{
			static_assert(TIsAnyOfV<TCheck, TErr...>);

			if (std::holds_alternative<TCheck>(m_Value))
			{
				lambda(std::get<TCheck>(m_Value));
				return true;
			}
			return false;
		}
	}

	// Final Result structs ---------------------------------------------------

	template<typename TRet, typename... TErr>
	struct Result : _Detail::ResultBase<TRet, TErr...>
	{
		/**
		 * @brief Construct a new Result object that holds the specified value
		 *
		 * @tparam T Value type, it must be one of the Result template types
		 * @param value The value
		 */
		template<typename T>
		Result(const T& value) :
			ResultBase(value)
		{
		}

		/**
		 * @brief Forward throw constructor
		 * Use fwdthrowall / mfwdthrowall macros for this.
		 * 
		 * @param fwdThrow Result to copy the error from.
		 */
		template<typename T>
		Result(T&& fwdThrow) :
			ResultBase(Move(fwdThrow))
		{
		}
	};

	template<typename... TErr>
	struct Result<void, TErr...> : _Detail::ResultBase<std::monostate, TErr...>
	{
		/**
		 * @brief Construct a new Result object that holds no value (void)
		 */
		Result() :
			ResultBase(std::monostate())
		{
		}

		/**
		 * @brief Construct a new Result object that holds the specified value
		 *
		 * @tparam T Value type, it must be one of the Result template types
		 * @param value The value
		 */
		template<typename T>
		Result(const T& value) :
			ResultBase(value)
		{
		}

		/**
		 * @brief Forward throw constructor
		 * Use fwdthrowall / mfwdthrowall macros for this.
		 * 
		 * @param fwdThrow Result to copy the error from.
		 */
		template<typename T>
		Result(T&& fwdThrow) :
			ResultBase(Move(fwdThrow))
		{
		}
	};

	/**
	 * @brief Use this to return the Result of void type at the end of the function
	 * return Void();
	 */
	using Void = std::monostate;

#pragma endregion

// Assertion macros ---------------------------------------------------------------------------------------

#pragma region Assertion / Error macros

/**
 * @brief Formats and prints the message and breaks the debugger.
 */
#define ionbreak(...) (void)((Ion::ErrorHandler::Break(__VA_ARGS__), 0) || (debugbreak(), 0))

#undef ionassert
#if ION_DEBUG && ION_ENABLE_DEBUG_ASSERTS
/**
 * @brief Break the debugger if the condition is not satisfied.
 * Does nothing on non-debug builds
 */
#define ionassert(x, ...) (void)(!!(x) || (Ion::ErrorHandler::AssertAbort(_PASS_ASSERT_ARGS(#x), __VA_ARGS__), 0) || (debugbreak(), 0))
#else
/**
 * @brief Break the debugger if the condition is not satisfied.
 * Does nothing on non-debug builds
 */
#define ionassert(x, ...) ((void)0)
#endif

/**
 * @brief Abort the program if the condition is not satisfied.
 */
#define ionverify(x, ...) (void)(!!(x) || (Ion::ErrorHandler::AssertAbort(_PASS_ASSERT_ARGS(#x), __VA_ARGS__), 0) || (debugbreak(), 0) || (abort(), 0))

// Throw macro -------------------------------------------------------------------------------------------------

	namespace _Detail
	{
		inline static String _FormatThrowMessage() { return ""; }
		template<typename... Args>
		inline static String _FormatThrowMessage(const String& format, Args&&... args) { return fmt::format(format, Forward<Args>(args)...); }
		template<typename... Args>
		inline static String _FormatThrowMessage(const WString& format, Args&&... args) { return StringConverter::WStringToString(fmt::format(format, Forward<Args>(args)...)); }
	}

#if ION_BREAK_ON_THROW
#define _breakthrow debugbreak();
#else
#define _breakthrow
#endif

#if ION_DEBUG
#define _ionthrow(error, ...) return [&](_ASSERT_ARGS) { auto err = error(Ion::_Detail::_FormatThrowMessage(__VA_ARGS__)); Ion::ErrorHandler::PrintErrorThrow(_FWD_ASSERT_ARGS, err); _breakthrow return err; }(_PASS_ASSERT_ARGS(0))
#else
#define _ionthrow(error, ...) return [&] { auto err = error(Ion::_Detail::_FormatThrowMessage(__VA_ARGS__)); _breakthrow return err; }()
#endif

/**
 * @brief Throws (returns) a Result type with a specified Error type as a value.
 * Always prints the Error in Debug configuration
 */
#define ionthrow(error, ...) _ionthrow(error, __VA_ARGS__)

/**
 * @brief Throws an Error if the condition is satisfied.
 */
#define ionthrowif(cond, error, ...) if ((cond)) ionthrow(error, __VA_ARGS__)

#define _fwdthrow(result, error) if (result.Is<error>()) return Move(result.ForwardThrow<error>())
#define _fwdthrowall(result) if (!result) return Move(result)

/**
 * @brief Forwards the Error throw if the Result is of its type.
 */
#define fwdthrow(result, error) { auto& R = result; _fwdthrow(R, error); }
/**
 * @brief Always forwards the Error throw.
 */
#define fwdthrowall(result) { auto& R = result; _fwdthrowall(R); }

// Safe unwrap --------------------------------------------------------------------------------------------------

/**
 * @brief If the result is Ok, unwrap it, else forward throw.
 * 
 * @param var Variable to store the Ok variant in
 * @param result Result to unwrap
 */
#define safe_unwrap(var, result) { auto R = result; if (R) var = R.Unwrap(); else return Move(R); }

// Result match -------------------------------------------------------------------------------------------------

/**
 * @brief A pattern for handling errors.
 * Creates a scope with variable R that holds the Result type.
 */
#define ionmatchresult(result, cases) { auto R = result; if (0){} cases }

/**
 * @brief Match on the specified type
 */
#define mcase(type) else if (R.Is<type>())
/**
 * @brief Match on the Ok variant
 */
#define mcaseok else if (R)
/**
 * @brief Match on all Err variants
 */
#define mcaseerr else if (!R)
/**
 * @brief Match on the specified Error type and forward throw
 */
#define mfwdthrow(error) else _fwdthrow(R, error);
/**
 * @brief Match on all the Error types and forward throw
 */
#define mfwdthrowall else _fwdthrowall(R);
/**
 * @brief Match on all other variants
 */
#define melse else

// Error macro -------------------------------------------------------------------------------------------------

/**
 * @brief Throws an irrecoverable error.
 */
#define ionerror(error, ...) (void)((Ion::ErrorHandler::ErrorAbort(_PASS_ASSERT_ARGS(0), error(Ion::_Detail::_FormatThrowMessage(__VA_ARGS__))), 0) || (debugbreak(), 0) || (abort(), 0))

#pragma endregion

}
