#pragma once

#include "CoreApi.h"
#include "CoreMacros.h"

namespace Ion
{
	class ION_API StringConverter
	{
	public:
		/* Converts a Wide String to Multi-Byte String using UTF-8 code page. */
		NODISCARD inline static String WStringToString(const WString& wstring)
		{
			uint64 length = wstring.size();
			char* cstr = new char[(length + 1) * sizeof(char)];
			WCharToChar(wstring.c_str(), cstr, (int32)length);
			String string(cstr);
			delete[] cstr;
			return std::move(string);
		}

		/* Converts a Multi-Byte String to Wide String using UTF-8 code page. */
		NODISCARD inline static WString StringToWString(const String& string)
		{
			uint64 length = string.size();
			wchar* wcstr = new wchar[(length + 1) * sizeof(wchar)];
			CharToWChar(string.c_str(), wcstr, (int32)length);
			WString wstring(wcstr);
			delete[] wcstr;
			return std::move(wstring);
		}

		static int32 WCharToChar(const wchar* wcstr, char* outCstr, int32 cstrLength);
		static int32 CharToWChar(const char* cstr, wchar* outWcstr, int32 wcstrLength);

		template<size_t Size>
		inline static int32 WCharToChar(const wchar* wcstr, char (&outCstr)[Size])
		{
			return WCharToChar(wcstr, outCstr, Size - 1);
		}

		template<size_t Size>
		inline static int32 CharToWChar(const char* cstr, wchar(&outWcstr)[Size])
		{
			return CharToWChar(cstr, outWcstr, (Size / sizeof(wchar)) - 1);
		}

		template<typename T>
		NODISCARD inline static String ToString(T value)
		{
			return std::to_string(value);
		}

		template<typename T>
		NODISCARD inline static WString ToWString(T value)
		{
			return std::to_wstring(value);
		}
	};
}
