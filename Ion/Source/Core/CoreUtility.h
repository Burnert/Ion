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

template<typename T, typename U>
NODISCARD constexpr inline TEnableIfT<TIsIntegralV<T>, bool> GetBitflag(const T& bitmask, U bit) noexcept
{
	return (bitmask >> bit) & 1;
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

// Functional Bind:

template <typename F, typename... Types>
NODISCARD std::_Binder<std::_Unforced, F, Types...> Bind(F&& func, Types&&... args) {
	return std::bind(Forward<F>(func), Forward<Types>(args)...);
}

template <typename R, typename F, typename... Types>
NODISCARD std::_Binder<R, F, Types...> Bind(F&& func, Types&&... args) {
	return std::bind<R>(Forward<F>(func), Forward<Types>(args)...);
}

/** Bind placeholders */
namespace Placeholders
{
	inline constexpr std::_Ph<1>  P1  { };
	inline constexpr std::_Ph<2>  P2  { };
	inline constexpr std::_Ph<3>  P3  { };
	inline constexpr std::_Ph<4>  P4  { };
	inline constexpr std::_Ph<5>  P5  { };
	inline constexpr std::_Ph<6>  P6  { };
	inline constexpr std::_Ph<7>  P7  { };
	inline constexpr std::_Ph<8>  P8  { };
	inline constexpr std::_Ph<9>  P9  { };
	inline constexpr std::_Ph<10> P10 { };
	inline constexpr std::_Ph<11> P11 { };
	inline constexpr std::_Ph<12> P12 { };
	inline constexpr std::_Ph<13> P13 { };
	inline constexpr std::_Ph<14> P14 { };
	inline constexpr std::_Ph<15> P15 { };
	inline constexpr std::_Ph<16> P16 { };
	inline constexpr std::_Ph<17> P17 { };
	inline constexpr std::_Ph<18> P18 { };
	inline constexpr std::_Ph<19> P19 { };
	inline constexpr std::_Ph<20> P20 { };
}

// Macros for easy binding functions with unbound arguments

#define Bind_NoParams(fn) Bind(fn, this)
#define Bind_1Param(fn)   Bind(fn, this, Placeholders::P1)
#define Bind_2Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2)
#define Bind_3Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3)
#define Bind_4Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4)
#define Bind_5Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5)
#define Bind_6Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6)
#define Bind_7Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7)
#define Bind_8Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8)
#define Bind_9Params(fn)  Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9)
#define Bind_10Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10)
#define Bind_11Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11)
#define Bind_12Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12)
#define Bind_13Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13)
#define Bind_14Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13, Placeholders::P14)
#define Bind_15Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13, Placeholders::P14, Placeholders::P15)
#define Bind_16Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13, Placeholders::P14, Placeholders::P15, Placeholders::P16)
#define Bind_17Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13, Placeholders::P14, Placeholders::P15, Placeholders::P16, Placeholders::P17)
#define Bind_18Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13, Placeholders::P14, Placeholders::P15, Placeholders::P16, Placeholders::P17, Placeholders::P18)
#define Bind_19Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13, Placeholders::P14, Placeholders::P15, Placeholders::P16, Placeholders::P17, Placeholders::P18, Placeholders::P19)
#define Bind_20Params(fn) Bind(fn, this, Placeholders::P1, Placeholders::P2, Placeholders::P3, Placeholders::P4, Placeholders::P5, Placeholders::P6, Placeholders::P7, Placeholders::P8, Placeholders::P9, Placeholders::P10, Placeholders::P11, Placeholders::P12, Placeholders::P13, Placeholders::P14, Placeholders::P15, Placeholders::P16, Placeholders::P17, Placeholders::P18, Placeholders::P19, Placeholders::P20)

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
