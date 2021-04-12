#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreMacros.h"
#include "Core/Math/Math.h"

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
