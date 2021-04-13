#pragma once

#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"
#include "Templates/Templates.h"

// Bitwise :

NODISCARD constexpr uint Bitflag(ubyte bit) noexcept { return 1u << bit; }

template<typename T, typename U, 
	TEnableIfT<TIsIntegralV<T>>* = nullptr,
	TEnableIfT<TIsIntegralV<U>>* = nullptr>
NODISCARD inline void SetBitflags(T& bitmask, const U& bits) noexcept
{
	bitmask |= bits;
}

template<typename T, typename U,
	TEnableIfT<TIsIntegralV<T>>* = nullptr,
	TEnableIfT<TIsIntegralV<U>>* = nullptr>
NODISCARD inline void UnsetBitflags(T& bitmask, const U& bits) noexcept
{
	bitmask &= ~(bits);
}

template<typename T, typename U,
	TEnableIfT<TIsIntegralV<T>>* = nullptr,
	TEnableIfT<TIsIntegralV<U>>* = nullptr>
NODISCARD inline void ToggleBitflags(T& bitmask, const U& bits) noexcept
{
	bitmask ^= bits;
}

template<typename T, typename U,
	TEnableIfT<TIsIntegralV<T>>* = nullptr,
	TEnableIfT<TIsIntegralV<U>>* = nullptr>
NODISCARD constexpr inline U GetBitflags(const T& bitmask, const U& bits) noexcept
{
	return bitmask & bits;
}

template<typename T>
NODISCARD constexpr inline TEnableIfT<TIsIntegralV<T>, bool> GetBitflag(const T& bitmask, ubyte bit) noexcept
{
	return (bitmask >> bit) & 0x01u;
}

template<typename T>
NODISCARD constexpr inline TEnableIfT<TIsIntegralV<T>, T> BooleanToBitmask(bool boolean) noexcept
{
	return (T)-(llong)boolean;
}

// Memory :

template<typename T, typename... Types>
NODISCARD constexpr FORCEINLINE TShared<T> MakeShared(Types&&... args)
{
	return std::make_shared<T>(std::forward<Types>(args)...);
}

template<typename T>
NODISCARD constexpr FORCEINLINE TShared<T> MakeShareable(T* ptr)
{
	return TShared<T>(ptr);
}

template<typename T, typename... Types>
NODISCARD constexpr FORCEINLINE TUnique<T> MakeUnique(Types&&... args)
{
	return std::make_unique<T>(std::forward<Types>(args)...);
}

// Common:

template<typename T, typename... Elements>
NODISCARD inline constexpr bool IsAnyOf(T&& item, Elements&&... elements)
{
	return ((item == elements) || ...);
}
