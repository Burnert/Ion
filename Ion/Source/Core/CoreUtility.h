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

/* Templated version of FLAGS_IF()
   If the condition is true, return the bitflags, else return 0. */
template<typename T>
NODISCARD constexpr inline TIf<TIsEnumV<T>, uint32, T> FlagsIf(bool condition, T bitflags) noexcept
{
	using Type = TIf<TIsEnumV<T>, uint32, T>;
	static_assert(TIsIntegralV<Type>);
	return ~((Type)condition - 1) & bitflags;
}

// Memory :

template<typename SizeT, typename AlignT>
NODISCARD constexpr SizeT AlignAs(SizeT size, AlignT align)
{
	static_assert(TIsNoneOfV<SizeT, bool>);
	static_assert(TIsIntegralV<SizeT>);
	static_assert(TIsNoneOfV<AlignT, bool>);
	static_assert(TIsIntegralV<AlignT>);

	return ((size + align - 1) / align) * align;
}

template<typename T, typename... Types>
NODISCARD constexpr TShared<T> MakeShared(Types&&... args)
{
	return std::make_shared<T>(std::forward<Types>(args)...);
}

template<typename T>
NODISCARD constexpr TShared<T> MakeShareable(T* ptr)
{
	return TShared<T>(ptr);
}

template<typename T, typename... Types>
NODISCARD constexpr TUnique<T> MakeUnique(Types&&... args)
{
	return std::make_unique<T>(std::forward<Types>(args)...);
}

template<typename T>
NODISCARD constexpr TRemoveRef<T>&& Move(T&& arg) noexcept
{
	return static_cast<TRemoveRef<T>&&>(arg);
}

// forward an lvalue as either an lvalue or an rvalue
template<typename T>
NODISCARD constexpr T&& Forward(TRemoveRef<T>& arg) noexcept
{
	return static_cast<T&&>(arg);
}

// forward an rvalue as an rvalue
template<typename T>
NODISCARD constexpr T&& Forward(TRemoveRef<T>&& arg) noexcept
{
	static_assert(!std::is_lvalue_reference_v<T>, "bad forward call");
	return static_cast<T&&>(arg);
}

template<typename T1, typename T2>
NODISCARD TShared<T1> TStaticCast(const TShared<T2>& other) noexcept
{
	return std::static_pointer_cast<T1>(other);
}

template<typename T1, typename T2>
NODISCARD TShared<T1> TStaticCast(TShared<T2>&& other) noexcept
{
	return std::static_pointer_cast<T1>(other);
}

// Common:

//template<typename T, T... Elements>
//struct StaticArray;
//
//template<typename T>
//struct StaticArray<T> { };
//
//template<typename T, T First, T... Rest>
//struct StaticArray<T, First, Rest...>
//{
//	using Type = T;
//
//	constexpr static Type Element = First;
//	constexpr static StaticArray<T, Rest...> OtherElements = StaticArray<T, Rest...>();
//};

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
