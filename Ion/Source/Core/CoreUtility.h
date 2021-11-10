#pragma once

#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"
#include "Templates/Templates.h"

// Bitwise :

NODISCARD constexpr uint32 Bitflag(uint8 bit) noexcept { return 1u << bit; }

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
NODISCARD constexpr inline TEnableIfT<TIsIntegralV<T>, bool> GetBitflag(const T& bitmask, uint8 bit) noexcept
{
	return (bitmask >> bit) & 0x01u;
}

template<typename T>
NODISCARD constexpr inline TEnableIfT<TIsIntegralV<T>, T> BooleanToBitmask(bool boolean) noexcept
{
	return (T)-(int64)boolean;
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

template<typename T>
NODISCARD constexpr FORCEINLINE TRemoveRef<T>&& Move(T&& arg) noexcept
{
	return static_cast<TRemoveRef<T>&&>(arg);
}

// Thread:



// Common:

template<typename T, T... Elements>
struct StaticArray;

template<typename T>
struct StaticArray<T> { };

template<typename T, T First, T... Rest>
struct StaticArray<T, First, Rest...>
{
	using Type = T;

	constexpr static Type Element = First;
	constexpr static StaticArray<T, Rest...> OtherElements = StaticArray<T, Rest...>();
};

template<typename T, typename U, uint64 Size>
NODISCARD inline constexpr bool IsAnyOf(T&& item, const U(&elements)[Size])
{
	for (uint64 i = 0; i < Size; ++i)
	{
		if (elements[i] == item)
			return true;
	}
	return false;
}

template<typename T, typename... Elements>
NODISCARD inline constexpr bool IsAnyOf(T&& item, Elements&&... elements)
{
	return ((item == elements) || ...);
}

template<typename T, typename U, uint64 Size>
NODISCARD inline constexpr bool IsNoneOf(T&& item, const U(&elements)[Size])
{
	return !IsAnyOf(item, elements);
}

template<typename T, typename... Elements>
NODISCARD inline constexpr bool IsNoneOf(T&& item, Elements&&... elements)
{
	return !IsAnyOf(item, elements...);
}
