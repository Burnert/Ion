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
inline static int tstrcmp(const T* s1, const T* s2) { return 0; }
template<> inline static int tstrcmp(const char* s1, const char* s2)   { return strcmp(s1, s2); }
template<> inline static int tstrcmp(const wchar* s1, const wchar* s2) { return wcscmp(s1, s2); }

template<typename T>
inline static uint64 tstrlen(const T* s) { return 0; }
template<> inline static uint64 tstrlen(const char* s)  { return strlen(s); }
template<> inline static uint64 tstrlen(const wchar* s) { return wcslen(s); }


// Type definition to String converter functions

template<typename Type>
NODISCARD FORCEINLINE static constexpr const char* TypeToString()                             { return "Undefined"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<bool>()            { return "Boolean"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<char>()            { return "Char"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<ubyte>()           { return "UnsignedChar"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<short>()           { return "Short"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<ushort>()          { return "UnsignedShort"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<int>()             { return "Integer"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<uint>()            { return "UnsignedInteger"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<long>()            { return "Long"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<ulong>()           { return "UnsignedLong"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<llong>()           { return "LongLong"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<ullong>()          { return "UnsignedLongLong"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<float>()           { return "Float"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<double>()          { return "Double"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<String>()          { return "String"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<WString>()         { return "WideString"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FVector2>()   { return "FloatVector2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FVector3>()   { return "FloatVector3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FVector4>()   { return "FloatVector4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::IVector2>()   { return "IntegerVector2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::IVector3>()   { return "IntegerVector3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::IVector4>()   { return "IntegerVector4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix2>()   { return "FloatMatrix2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix2x3>() { return "FloatMatrix2x3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix2x4>() { return "FloatMatrix2x4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix3>()   { return "FloatMatrix3"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix3x2>() { return "FloatMatrix3x2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix3x4>() { return "FloatMatrix3x4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix4>()   { return "FloatMatrix4"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix4x2>() { return "FloatMatrix4x2"; }
template<> NODISCARD FORCEINLINE static constexpr const char* TypeToString<Ion::FMatrix4x3>() { return "FloatMatrix4x3"; }
