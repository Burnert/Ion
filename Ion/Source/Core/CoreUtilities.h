#pragma once

#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"

NODISCARD constexpr uint Bitflag(byte bit) noexcept { return 1u << bit; }

template<typename T, typename U, 
	std::enable_if_t<std::is_integral_v<T>>* = nullptr,
	std::enable_if_t<std::is_integral_v<U>>* = nullptr>
NODISCARD inline void SetBitflags(T& bitmask, const U& bits) noexcept
{
	bitmask |= bits;
}

template<typename T, typename U,
	std::enable_if_t<std::is_integral_v<T>>* = nullptr,
	std::enable_if_t<std::is_integral_v<U>>* = nullptr>
NODISCARD inline void UnsetBitflags(T& bitmask, const U& bits) noexcept
{
	bitmask &= ~(bits);
}

template<typename T, typename U,
	std::enable_if_t<std::is_integral_v<T>>* = nullptr,
	std::enable_if_t<std::is_integral_v<U>>* = nullptr>
NODISCARD inline void ToggleBitflags(T& bitmask, const U& bits) noexcept
{
	bitmask ^= bits;
}

template<typename T, typename U,
	std::enable_if_t<std::is_integral_v<T>>* = nullptr,
	std::enable_if_t<std::is_integral_v<U>>* = nullptr>
NODISCARD constexpr inline U GetBitflags(const T& bitmask, const U& bits) noexcept
{
	return bitmask & bits;
}

template<typename T>
NODISCARD constexpr inline std::enable_if_t<std::is_integral_v<T>, bool> GetBitflag(const T& bitmask, byte bit) noexcept
{
	return (bitmask >> bit) & 0x01u;
}

template<typename T>
NODISCARD constexpr inline std::enable_if_t<std::is_integral_v<T>, T> BooleanToBitmask(bool boolean) noexcept
{
	return (T)-(llong)boolean;
}
