#pragma once

#include "Core/Base.h"
#include "Core/Math/Math.h"

// Convert c-string literal to a specified type

template<typename T>
constexpr const T* StringLiteralAs(char* str, wchar* wstr);
template<>
constexpr const char* StringLiteralAs(char* str, wchar* wstr) { return str; }
template<>
constexpr const wchar* StringLiteralAs(char* str, wchar* wstr) { return wstr; }

#define STR_LITERAL_AS(str, type) StringLiteralAs<type>(str, L ## str)


// Templated c-string functions

template<typename T>
inline static int32 tstrcmp(const T* s1, const T* s2) { return 0; }
template<> inline static int32 tstrcmp<char>(const char* s1, const char* s2)   { return strcmp(s1, s2); }
template<> inline static int32 tstrcmp<wchar>(const wchar* s1, const wchar* s2) { return wcscmp(s1, s2); }

template<typename T>
inline static uint64 tstrlen(const T* s) { return 0; }
template<> inline static uint64 tstrlen<char>(const char* s)  { return strlen(s); }
template<> inline static uint64 tstrlen<wchar>(const wchar* s) { return wcslen(s); }

FORCEINLINE constexpr const char* BoolStr(bool value) { return value ? "true" : "false"; }
FORCEINLINE constexpr const wchar* BoolWStr(bool value) { return value ? L"true" : L"false"; }

// Comparison

/**
 * @brief Case insensitive string comparison
 */
template<typename T1, typename T2>
static bool EqualsCI(const T1& lhs, const T2& rhs)
{
	return std::equal(
		std::begin(lhs), std::end(lhs),
		std::begin(rhs), std::end(rhs),
		[](char a, char b) {
			return std::tolower((unsigned char)a) == std::tolower((unsigned char)b);
		});
}

// Typical string operations ---------------------------------------------------------------

#define _MINIMUM_SPLIT_ARRAY_N     5
#define _MINIMUM_SPLIT_STRING_SIZE 8

template<typename T>
inline static TArray<std::basic_string<T>> SplitString(const std::basic_string<T>& str, T delimiter)
{
	TArray<std::basic_string<T>> split;
	if (str.empty())
		return split;

	split.reserve(_MINIMUM_SPLIT_ARRAY_N);

	split.emplace_back().reserve(_MINIMUM_SPLIT_STRING_SIZE);
	for (auto& c : str)
	{
		if (c == delimiter)
		{
			split.emplace_back().reserve(_MINIMUM_SPLIT_STRING_SIZE);
			continue;
		}

		split.back() += c;
	}

	return split;
}

template<typename T>
inline static TArray<std::basic_string<T>> SplitString(const std::basic_string<T>& str, std::initializer_list<T> delimiters)
{
	TArray<std::basic_string<T>> split;
	if (str.empty())
		return split;

	split.reserve(_MINIMUM_SPLIT_ARRAY_N);

	split.emplace_back().reserve(_MINIMUM_SPLIT_STRING_SIZE);
	for (auto& c : str)
	{
		if (IsAnyOf(c, delimiters))
		{
			split.emplace_back().reserve(_MINIMUM_SPLIT_STRING_SIZE);
			continue;
		}

		split.back() += c;
	}

	return split;
}

template<typename T>
inline static TArray<std::basic_string<T>> SplitString(const std::basic_string<T>& str, const std::basic_string<T>& longDelimiter)
{
	TArray<std::basic_string<T>> split;
	if (str.empty() || longDelimiter.empty())
		return split;

	split.reserve(_MINIMUM_SPLIT_ARRAY_N);

	auto begin = str.begin();
	while (begin < str.end())
	{
		// Keep finding the delimeter and push the substring before it into the array.
		size_t offset = begin - str.begin();
		size_t found = str.find(longDelimiter, offset);
		auto end = found != std::basic_string<T>::npos ? str.begin() + found : str.end();
		size_t count = end - begin;
		split.emplace_back(str.substr(offset, count));
		begin = end == str.end() ? str.end() : end + longDelimiter.size();
		// Add an empty string if there was a delimiter at the end
		if (found != std::basic_string<T>::npos && begin == str.end())
			split.emplace_back();
	}

	return split;
}

template<typename T>
inline static String JoinString(const TArray<std::basic_string<T>>& strArray, T delimiter)
{
	String joined;
	joined.reserve(_MINIMUM_SPLIT_ARRAY_N * _MINIMUM_SPLIT_STRING_SIZE);

	for (auto it = strArray.cbegin(); it != strArray.cend(); ++it)
	{
		joined += *it;
		if (it < strArray.cend() - 1)
			joined += delimiter;
	}

	return joined;
}

template<typename T>
inline static String JoinString(const TArray<std::basic_string<T>>& strArray, const std::basic_string<T>& delimiter)
{
	String joined;
	joined.reserve(_MINIMUM_SPLIT_ARRAY_N * _MINIMUM_SPLIT_STRING_SIZE);

	for (auto it = strArray.cbegin(); it != strArray.cend(); ++it)
	{
		joined += *it;
		if (it < strArray.cend() - 1)
			joined += delimiter;
	}

	return joined;
}

// Type definition to String converter functions

template<typename Type>
NODISCARD FORCEINLINE static constexpr const char* TypeToString()                             { return "Undefined"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<bool>()            { return "Boolean"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<int8>()            { return "Integer8"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<uint8>()           { return "UnsignedInteger8"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<int16>()           { return "Integer16"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<uint16>()          { return "UnsignedInteger16"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<int32>()           { return "Integer32"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<uint32>()          { return "UnsignedInteger32"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<int64>()           { return "Integer64"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<uint64>()          { return "UnsignedInteger64"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<long>()            { return "Long"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<unsigned long>()   { return "UnsignedLong"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<float>()           { return "Float"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<double>()          { return "Double"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<wchar>()           { return "WideChar"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<String>()          { return "String"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<WString>()         { return "WideString"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Vector2>()   { return "FloatVector2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Vector3>()   { return "FloatVector3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Vector4>()   { return "FloatVector4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::IVector2>()  { return "IntegerVector2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::IVector3>()  { return "IntegerVector3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::IVector4>()  { return "IntegerVector4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix2>()   { return "FloatMatrix2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix2x3>() { return "FloatMatrix2x3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix2x4>() { return "FloatMatrix2x4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix3>()   { return "FloatMatrix3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix3x2>() { return "FloatMatrix3x2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix3x4>() { return "FloatMatrix3x4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix4>()   { return "FloatMatrix4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix4x2>() { return "FloatMatrix4x2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::Matrix4x3>() { return "FloatMatrix4x3"; }

template<typename T>
NODISCARD FORCEINLINE constexpr String ToString(T value)
{
	return std::to_string(value);
}
template<>
NODISCARD FORCEINLINE String ToString(bool value)
{
	return BoolStr(value);
}

template<typename T>
NODISCARD FORCEINLINE constexpr WString ToWString(T value)
{
	return std::to_wstring(value);
}
template<>
NODISCARD FORCEINLINE WString ToWString(bool value)
{
	return BoolWStr(value);
}

// Hex conversion -------------------------------------------------------------------------

NODISCARD FORCEINLINE constexpr char HexFrom4Bytes(uint8 bytes)
{
	ionassert(bytes < 16);
	if (bytes >= 16)
		return '\0';
	if (bytes < 10)
		return '0' + bytes;
	else
		return 'a' + (bytes - 10);
}

template<typename T>
NODISCARD FORCEINLINE String ToHex(T value) { return ""; }
template<> NODISCARD FORCEINLINE String ToHex(uint8 value)
{
	uint8 lsbits = value & 0x0F;
	uint8 msbits = (value >> 4) & 0x0F;
	String hex;
	hex.reserve(2);
	hex += HexFrom4Bytes(msbits);
	hex += HexFrom4Bytes(lsbits);
	return hex;
}
template<> NODISCARD FORCEINLINE String ToHex(int8 value) { return ToHex((uint8)value); }
template<> NODISCARD FORCEINLINE String ToHex(uint16 value)
{
	uint8 lsb = value & 0xFF;
	uint8 msb = (value >> 8) & 0xFF;
	String hex;
	hex.reserve(4);
	hex += ToHex<uint8>(msb);
	hex += ToHex<uint8>(lsb);
	return hex;
}
template<> NODISCARD FORCEINLINE String ToHex(int16 value) { return ToHex((uint16)value); }
template<> NODISCARD FORCEINLINE String ToHex(uint32 value)
{
	uint16 lsb = value & 0xFFFF;
	uint16 msb = (value >> 16) & 0xFFFF;
	String hex;
	hex.reserve(8);
	hex += ToHex<uint16>(msb);
	hex += ToHex<uint16>(lsb);
	return hex;
}
template<> NODISCARD FORCEINLINE String ToHex(int32 value) { return ToHex((uint32)value); }
template<> NODISCARD FORCEINLINE String ToHex(uint64 value)
{
	uint32 lsb = value & 0xFFFFFFFF;
	uint32 msb = (value >> 32) & 0xFFFFFFFF;
	String hex;
	hex.reserve(16);
	hex += ToHex<uint32>(msb);
	hex += ToHex<uint32>(lsb);
	return hex;
}
template<> NODISCARD FORCEINLINE String ToHex(int64 value) { return ToHex((uint64)value); }
