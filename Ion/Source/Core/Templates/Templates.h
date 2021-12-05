#pragma once

#include "StandardWrappers.h"

template<typename T, typename U>
inline constexpr bool _IsGreaterThan(T val, U thanVal)
{
	return val > thanVal;
}

template<typename T, typename U>
inline constexpr bool _IsLessThan(T val, U thanVal)
{
	return val < thanVal;
}

// Null type

struct TNull { };

// Value stored as a type

template<auto V>
struct TValue
{
	static constexpr auto Value = V;
};

// Type Pack --------------------------------------------------------------

template<typename... Types>
struct TTypePack;

template<>
struct TTypePack<>
{
	using ThisType = TNull;
	using RestTypes = TNull;
};

template<typename Type, typename... Rest>
struct TTypePack<Type, Rest...>
{
	using ThisType = Type;
	using RestTypes = TTypePack<Rest...>;
};

//template<typename T, typename... Types>
//struct TIsAnyOf { };
//
//template<typename T, typename... Types>
//struct TIsAnyOf<T, TTypePack<Types...>>
//{
//	static constexpr bool Value = TOrV<TIsSame<T, Types>...>;
//};

template<typename T, typename... Types>
inline constexpr bool TIsAnyOfV<T, TTypePack<Types...>> = TOrV<TIsSame<T, Types>...>;

template<typename T, typename... Types>
inline constexpr bool TIsNoneOfV<T, TTypePack<Types...>> = !TIsAnyOfV<T, TTypePack<Types...>>;

// Template Max ----------------------------------------------------------

template<auto V1, auto V2, typename EnableT = void>
struct TMax { };

template<auto V1, auto V2>
struct TMax<V1, V2, typename TEnableIfT<_IsGreaterThan(V1, V2)>>
{
	static constexpr auto Value = V1;
};

template<auto V1, auto V2>
struct TMax<V1, V2, typename TEnableIfT<!_IsGreaterThan(V1, V2)>>
{
	static constexpr auto Value = V2;
};

// Template Min ----------------------------------------------------------

template<auto V1, auto V2, typename EnableT = void>
struct TMin { };

template<auto V1, auto V2>
struct TMin<V1, V2, typename TEnableIfT<_IsLessThan(V1, V2)>>
{
	static constexpr auto Value = V1;
};

template<auto V1, auto V2>
struct TMin<V1, V2, typename TEnableIfT<!_IsLessThan(V1, V2)>>
{
	static constexpr auto Value = V2;
};

// Template MaxOfAll ----------------------------------------------------------

template<auto... Values>
struct TMaxOfAll { };

template<>
struct TMaxOfAll<>
{
	using Type = TNull;

	static constexpr auto Value = 0;
};

template<auto V, auto... VRest>
struct TMaxOfAll<V, VRest...>
{
	using Type = decltype(V);
	using MaxRest = TMaxOfAll<VRest...>;

	static constexpr auto Value =
		TIf<TIsDifferentV<MaxRest::Type, TNull>,
			TMax<V, MaxRest::Value>,
			TValue<V>
		>::Value;
};

// Template MinOfAll ----------------------------------------------------------

template<auto... Values>
struct TMinOfAll { };

template<>
struct TMinOfAll<>
{
	using Type = TNull;

	static constexpr auto Value = 0;
};

template<auto V, auto... VRest>
struct TMinOfAll<V, VRest...>
{
	using Type = decltype(V);
	using MinRest = TMinOfAll<VRest...>;

	static constexpr auto Value =
		TIf<TIsDifferentV<MinRest::Type, TNull>,
			TMin<V, MinRest::Value>,
			TValue<V>
		>::Value;
};

// Size Of Multiple Types Helper ----------------------------------------------------------

template<typename... Types>
struct TTypeSizeHelper { };

template<>
struct TTypeSizeHelper<>
{
	static constexpr size_t Max = 0;
	static constexpr size_t Min = TNumericLimits<size_t>::max();
};

template<typename Type, typename... Rest>
struct TTypeSizeHelper<Type, Rest...>
{
	using SizeRest = TTypeSizeHelper<Rest...>;
	
	static constexpr size_t Max = TMax<sizeof(Type), SizeRest::Max>::Value;
	static constexpr size_t Min = TMin<sizeof(Type), SizeRest::Min>::Value;
};

// TTypeSizeHelper specialization for TTypePack -----------------------------------------
// @TODO: Write why this works, because it's pretty ingenious,
// and it'll work for every template based on the same concept as this one

template<>
struct TTypeSizeHelper<TTypePack<>> : public TTypeSizeHelper<> { };

template<typename... Types>
struct TTypeSizeHelper<TTypePack<Types...>> : public TTypeSizeHelper<
	typename TTypePack<Types...>::ThisType, 
	typename TTypePack<Types...>::RestTypes> { };
