#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreMacros.h"
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
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<ulong>()           { return "UnsignedLong"; }
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
