#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"

namespace Ion
{
// Error handler ----------------------------------------------------------------------------------------

#define _ASSERT_ARGS const char* expr, const char* func, const char* file, int32 line
#define _FWD_ASSERT_ARGS expr, func, file, line
#define _PASS_ASSERT_ARGS(expr) expr, __FUNCSIG__, __FILE__, __LINE__

#define _ABORT_LOG_PATTERN        "({0}) => false\nFunction: {1}\nAt {2}:{3}\n"
#define _ABORT_LOG_PATTERN_NOEXPR "Function: {1}\nAt {2}:{3}\n"

	struct ErrorHandler
	{
		template<typename... Args>
		static void AssertAbort(_ASSERT_ARGS, const char* format = nullptr, Args&&... args);

		template<typename TResult>
		static void UnwrapAbort(_ASSERT_ARGS, const TResult& result);

		ErrorHandler() = delete;
	};

	template<typename... Args>
	inline void ErrorHandler::AssertAbort(_ASSERT_ARGS, const char* format, Args&&... args)
	{
		LOG_CRITICAL("Critical error has occured!");
		LOG_CRITICAL("Assertion failed: \n" _ABORT_LOG_PATTERN, _FWD_ASSERT_ARGS);
		if (format)
			LOG_CRITICAL(format, args...);
	}

	template<typename TResult>
	inline void ErrorHandler::UnwrapAbort(_ASSERT_ARGS, const TResult& result)
	{

		LOG_CRITICAL("Critical error has occured!");
		LOG_CRITICAL("Unwrap failed: => Result<{4}>\n" _ABORT_LOG_PATTERN_NOEXPR, _FWD_ASSERT_ARGS, result.GetErrorClassName());
#if ION_DEBUG
		String message = result.GetErrorMessage();
		if (!message.empty())
			LOG_CRITICAL(message);
#endif
	}

// Error struct -------------------------------------------------------------------------------------------

#if ION_DEBUG
#define _ERROR_TYPE_CTOR_HELPER(type) type(const String& message) : Error(message) { } type() : Error() { }
#else
#define _ERROR_TYPE_CTOR_HELPER(type) type() : Error() { }
#endif

#define ERROR_TYPE(type) struct type : Error { static constexpr const char* ClassName = #type; _ERROR_TYPE_CTOR_HELPER(type) }

	struct Error
	{
		static constexpr const char* ClassName = "Error";
#if ION_DEBUG
		const String Message;

		Error(const String& message) :
			Message(message)
		{
		}
#endif
		Error()
#if ION_DEBUG
			: Message()
#endif
		{
		}
	};

// Engine Error types --------------------------------------

	ERROR_TYPE(IOError);
	ERROR_TYPE(FileNotFoundError);

// Result struct -----------------------------------------------------------------------------------

	namespace _Detail
	{
		template<typename TRet, typename... TErr>
		struct ResultBase
		{
			static_assert(!TIsBaseOfV<TRet, Error>, "Primary Result type cannot be derived from Error.");
			static_assert((TIsBaseOfV<Error, TErr> && ...), "All secondary Result types must be derived from Error.");

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
			ResultBase& Get(F lambda);
			template<typename E, typename F>
			ResultBase& Err(F lambda);

			bool OK() const;

			ResultBase(const ResultBase&) = delete;
			ResultBase(ResultBase&&) = delete;
			ResultBase& operator=(const ResultBase&) = delete;
			ResultBase& operator=(ResultBase&&) = delete;

		private:
			String GetErrorClassName() const;
#if ION_DEBUG
			String GetErrorMessage() const;
#endif

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
			if (!std::holds_alternative<TRet>(m_Value))
				return fallback;

			return std::get<TRet>(m_Value);
		}

		template<typename TRet, typename... TErr>
		template<typename F>
		inline ResultBase<TRet, TErr...>& ResultBase<TRet, TErr...>::Get(F lambda)
		{
			static_assert(TIsConvertibleV<F, TFunction<void(const TRemoveConst<TRet>&)>>);

			if (std::holds_alternative<TRet>(m_Value))
				lambda(std::get<TRet>(m_Value));

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
		inline bool ResultBase<TRet, TErr...>::OK() const
		{
			return std::holds_alternative<TRet>(m_Value);
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

#if ION_DEBUG
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
#endif
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
			ResultBase(std::monostate)
		{
		}

		template<typename T>
		Result(const T& value) :
			ResultBase(value)
		{
		}
	};

// Assertion macros ----------------------------------------------------------------------------------------------

//#undef ionassert
//#define ionassert(x, ...) (void)(!!(x) || (Ion::ErrorHandler::AssertAbort(_PASS_ASSERT_ARGS(#x), __VA_ARGS__), 0) || (debugbreak(), 0))

#define ionverify(x, ...) (void)(!!(x) || (Ion::ErrorHandler::AssertAbort(_PASS_ASSERT_ARGS(#x), __VA_ARGS__), 0) || (abort(), 0))

// Throw macro -------------------------------------------------------------------------------------------------

	namespace _Detail
	{
		inline static String _FormatThrowMessage() { return ""; }
		template<typename... Args>
		inline static String _FormatThrowMessage(const String& format, Args&&... args) { return fmt::format(format, Forward<Args>(args)...); }
	}

#if ION_DEBUG
#define ionthrow(error, ...) return error(Ion::_Detail::_FormatThrowMessage(__VA_ARGS__))
#else
#define ionthrow(error, ...) return error()
#endif

}
