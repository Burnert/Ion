#pragma once

#include "CoreApi.h"
#include "CoreMacros.h"

namespace Ion
{
	class ION_API StringConverter
	{
	public:
		/**
		 * @brief Converts a Wide-Char string to a Multi-Byte string using a UTF-8 code page.
		 * 
		 * @param string Wide-Char string to convert from
		 * @return Converted Multi-Byte string in UTF-8 format
		 */
		NODISCARD inline static String WStringToString(const WString& string);

		/**
		 * @brief Converts a Multi-Byte string to a Wide-Char string using a UTF-8 code page.
		 * 
		 * @param string Multi-Byte string to convert from
		 * @return Converted Wide-Char string in UTF-8 format
		 */
		NODISCARD inline static WString StringToWString(const String& string);

		/**
		 * @brief Converts a Wide-Char string to a Multi-Byte string using a UTF-8 code page.
		 * 
		 * @param wcStr Wide-Char string to convert from
		 * 
		 * @param wcStrLen Length of the string in the wcStr parameter.
		 * If -1, this function will get the length based on the null character.
		 * 
		 * @param outBuffer Buffer to put the Wide-Char string in UTF-8 format into.
		 * If null, this function will return the minimum length (in characters) the buffer needs to be.
		 * 
		 * @param bufferCount Length of outBuffer. Ignored if outBuffer is null.
		 * 
		 * @return int32 Bytes written, or minimum buffer size (if outBuffer is null)
		 */
		static int32 W2MB(const wchar* wcStr, int32 wcStrLen, char* outBuffer, int32 bufferCount);

		template<size_t wcLen, size_t mbLen>
		static int32 W2MB(const wchar(&wcStr)[wcLen], char(&outBuffer)[mbLen]);

		/**
		 * @brief Converts a Multi-Byte string to a Wide-Char string using a UTF-8 code page.
		 * 
		 * @param mbStr Multi-Byte string to convert from
		 * 
		 * @param mbStrLen Length of the string in the mbStr parameter.
		 * If -1, this function will get the length based on the null character.
		 * 
		 * @param outBuffer Buffer to put the Multi-Byte string in UTF-8 format into.
		 * If null, this function will return the minimum length (in characters) the buffer needs to be.
		 * 
		 * @param bufferCount Length of outBuffer. Ignored if outBuffer is null.
		 * 
		 * @return int32 Bytes written, or minimum buffer size (if outBuffer is null)
		 */
		static int32 MB2W(const char* mbStr, int32 mbStrLen, wchar* outBuffer, int32 bufferCount);

		template<size_t mbLen, size_t wcLen>
		static int32 MB2W(const char(&mbStr)[mbLen], wchar(&outBuffer)[wcLen]);
	};

	inline String StringConverter::WStringToString(const WString& string)
	{
		if (string.empty())
			return EmptyString;

		int32 mbStrLength = W2MB(string.c_str(), (int32)string.size(), nullptr, 0);
		String mbStr(mbStrLength, 0);

		ionassert(mbStrLength < TNumericLimits<int32>::max(), "The string is too long.");

		int32 written = W2MB(string.c_str(), (int32)string.size(), mbStr.data(), mbStrLength);

		return mbStr;
	}

	inline WString StringConverter::StringToWString(const String& string)
	{
		if (string.empty())
			return EmptyWString;

		int32 wcStrLength = MB2W(string.c_str(), (int32)string.size(), nullptr, 0);
		WString wcStr(wcStrLength, 0);

		ionassert(wcStrLength < TNumericLimits<int32>::max(), "The string is too long.");

		int32 written = MB2W(string.c_str(), (int32)string.size(), wcStr.data(), wcStrLength);

		return wcStr;
	}

	template<size_t wcLen, size_t mbLen>
	inline static int32 StringConverter::W2MB(const wchar(&wcStr)[wcLen], char(&outBuffer)[mbLen])
	{
		return W2MB(wcStr, -1, outBuffer, mbLen);
	}

	template<size_t mbLen, size_t wcLen>
	inline static int32 StringConverter::MB2W(const char(&mbStr)[mbLen], wchar(&outBuffer)[wcLen])
	{
		return MB2W(mbStr, -1, outBuffer, wcLen);
	}
}
