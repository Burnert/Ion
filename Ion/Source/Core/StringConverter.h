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
			ullong length = wstring.size();
			char* cstr = new char[(length + 1) * sizeof(char)];
			WCharToChar(wstring.c_str(), cstr, (int)length);
			String string(cstr);
			delete[] cstr;
			return std::move(string);
		}

		/* Converts a Multi-Byte String to Wide String using UTF-8 code page. */
		NODISCARD inline static WString StringToWString(const String& string)
		{
			ullong length = string.size();
			wchar* wcstr = new wchar[(length + 1) * sizeof(wchar)];
			CharToWChar(string.c_str(), wcstr, (int)length);
			WString wstring(wcstr);
			delete[] wcstr;
			return std::move(wstring);
		}

		static int WCharToChar(const wchar* wcstr, char* outCstr, int cstrLength);
		static int CharToWChar(const char* cstr, wchar* outWcstr, int wcstrLength);
		
		template<size_t Size>
		inline static int WCharToChar(const wchar* wcstr, char (&outCstr)[Size])
		{
			return WCharToChar(wcstr, outCstr, Size - 1);
		}

		template<size_t Size>
		inline static int CharToWChar(const char* cstr, wchar(&outWcstr)[Size])
		{
			return CharToWChar(cstr, outWcstr, (Size / sizeof(wchar)) - 1);
		}
	};
}
