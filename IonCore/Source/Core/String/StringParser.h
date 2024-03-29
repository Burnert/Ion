#pragma once

#include "Core/Base.h"

namespace Ion
{
	// Enum-String conversion -------------------------------------------------------------------------------------

#define ENUM_PARSER_TO_STRING_BEGIN(type) inline static String ToString(type value) { switch (value) {
#define ENUM_PARSER_TO_STRING_HELPER(val) case decltype(value)::val: return #val;
#define ENUM_PARSER_TO_STRING_END() } CoreLogger.Error("Invalid enum value."); return ""; }

#define ENUM_PARSER_FROM_STRING_BEGIN(type) inline static TOptional<type> FromString(const String& str) { ionassert(!str.empty(), "The enum string is empty.");
#define ENUM_PARSER_FROM_STRING_HELPER(val) if (str == #val) return decltype(FromString(""))::value_type::val;
#define ENUM_PARSER_FROM_STRING_END() CoreLogger.Error("Invalid enum string."); return NullOpt; }

	template<typename TEnum>
	struct TEnumParser
	{
		inline static String ToString(TEnum value)
		{
			static_assert(false, "No specialization for type.");
		}

		inline static TOptional<TEnum> FromString(const String& str)
		{
			static_assert(false, "No specialization for type.");
		}
	};

	// String to type conversion --------------------------------------------------------------------------------

	template<typename T>
	struct TStringParser
	{
		inline TOptional<T> operator()(const String& str)
		{
			if constexpr (TIsEnumV<T>)
			{
				T val = TEnumParser<T>::FromString(str);
				if (!val)
				{
					CoreLogger.Error("Cannot parse an enum value.");
					return NullOpt;
				}
				return val;
			}
			else if constexpr (TIsSameV<T, bool>)
			{
				if (str == "true")
					return true;
				else if (str == "false")
					return false;

				CoreLogger.Error("Cannot parse a bool value. -> {0}", str);
				return NullOpt;
			}
			else if constexpr (TIsIntegralV<T>)
			{
				char* end;
				T val = tstrtoi<T>(str.c_str(), &end, 10);
				if (end == str.c_str() || errno == ERANGE)
				{
					CoreLogger.Error("Cannot parse an integral value. -> {0}", str);
					return NullOpt;
				}
				return val;
			}
			else if constexpr (TIsFloatingV<T>)
			{
				char* end;
				T val = tstrtof<T>(str.c_str(), &end);
				if (end == str.c_str() || errno == ERANGE)
				{
					CoreLogger.Error("Cannot parse a floating-point value. -> {0}", str);
					return NullOpt;
				}
				return val;
			}
			else if constexpr (TIsSameV<T, GUID>)
			{
				ionmatchresult(GUID::FromString(str),
					mcaseok return R.Unwrap();
					melse
					{
						CoreLogger.Error("Cannot parse a GUID value. -> {0}", str);
						return NullOpt;
					}
				);
			}
			else
			{
				static_assert(false, "Bad type.");
				return NullOpt;
			}
		}

	private:
#define _tstrtoi_spec(t, f) template<> inline static t tstrtoi(const char* str, char** ppEnd, int radix) { return f(str, ppEnd, radix); }
		template<typename T>
		inline static T tstrtoi(const char* str, char** ppEnd, int radix)
		{
			int32 val = tstrtoi<int32>(str, ppEnd, radix);
			if (val > TNumericLimits<T>::max())
			{
				// Treat like a range error
				*ppEnd = const_cast<char*>(str);
				return (T)0;
			}
			return (T)val;
		}
		_tstrtoi_spec(int32,  strtol)
		_tstrtoi_spec(uint32, strtoul)
		_tstrtoi_spec(int64,  _strtoi64)
		_tstrtoi_spec(uint64, _strtoui64)

		template<typename T>
		inline static T tstrtof(const char* str, char** ppEnd) { static_assert(false); return 0; }
		
		template<> inline static float  tstrtof(const char* str, char** ppEnd) { return strtof(str, ppEnd); }
		template<> inline static double tstrtof(const char* str, char** ppEnd) { return strtod(str, ppEnd); }
	};
}
