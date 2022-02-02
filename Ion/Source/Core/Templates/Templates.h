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

template<auto... V>
struct TValue;

template<auto V>
struct TValue<V>
{
	using Type = decltype(V);

	static constexpr auto Value = V;
};

template<>
struct TValue<>
{
	using Type = TNull;
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

// Value Pack --------------------------------------------------------------

template<auto... Values>
struct TValuePack;

template<>
struct TValuePack<> { };

template<auto V, auto... VRest>
struct TValuePack<V, VRest...>
{
	using This = TValue<V>;
	using Rest = TValuePack<VRest...>;

	static constexpr auto ThisValue = This::Value;
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
inline constexpr bool TIsAnyOfV<T, TTypePack<Types...>> = TIsAnyOfV<T, Types...>;

template<typename T, typename... Types>
inline constexpr bool TIsNoneOfV<T, TTypePack<Types...>> = !TIsAnyOfV<T, TTypePack<Types...>>;

// Template Max Helpers ----------------------------------------------------------

namespace _THelpers
{
	template<auto V1, auto V2, typename EnableT = void>
	struct _TMax_2Val;

	template<auto V1, auto V2>
	struct _TMax_2Val<V1, V2, typename TEnableIfT<_IsGreaterThan(V1, V2)>>
	{
		static constexpr auto Value = V1;
	};

	template<auto V1, auto V2>
	struct _TMax_2Val<V1, V2, typename TEnableIfT<!_IsGreaterThan(V1, V2)>>
	{
		static constexpr auto Value = V2;
	};
}

// Template Max ----------------------------------------------------------

template<auto... Values>
struct TMax;

template<>
struct TMax<> { };

template<auto V>
struct TMax<V>
{
	static constexpr auto Value = V;
};

template<auto V1, auto V2>
struct TMax<V1, V2>
{
	static constexpr auto Value = _THelpers::_TMax_2Val<V1, V2>::Value;
};

template<auto V, auto... VRest>
struct TMax<V, VRest...>
{
	static constexpr auto Value = _THelpers::_TMax_2Val<V, TMax<VRest...>::Value>::Value;
};

template<auto... Values>
inline constexpr auto TMaxV = TMax<Values...>::Value;

// Template Max Of Value Pack ----------------------------------------------------------

template<typename PackT>
struct TMaxPack;

template<auto... Values>
struct TMaxPack<TValuePack<Values...>>
{
	static constexpr auto Value = TMax<Values...>::Value;
};

template<typename PackT>
inline constexpr auto TMaxPackV = TMaxPack<PackT>::Value;

// Template Min Helpers ----------------------------------------------------------

namespace _THelpers
{
	template<auto V1, auto V2, typename EnableT = void>
	struct _TMin_2Val;

	template<auto V1, auto V2>
	struct _TMin_2Val<V1, V2, typename TEnableIfT<_IsLessThan(V1, V2)>>
	{
		static constexpr auto Value = V1;
	};

	template<auto V1, auto V2>
	struct _TMin_2Val<V1, V2, typename TEnableIfT<!_IsLessThan(V1, V2)>>
	{
		static constexpr auto Value = V2;
	};
}

// Template Min ----------------------------------------------------------

template<auto... Values>
struct TMin;

template<>
struct TMin<> { };

template<auto V>
struct TMin<V>
{
	static constexpr auto Value = V;
};

template<auto V1, auto V2>
struct TMin<V1, V2>
{
	static constexpr auto Value = _THelpers::_TMin_2Val<V1, V2>::Value;
};

template<auto V, auto... VRest>
struct TMin<V, VRest...>
{
	static constexpr auto Value = _THelpers::_TMin_2Val<V, TMin<VRest...>::Value>::Value;
};

template<auto... Values>
inline constexpr auto TMinV = TMin<Values...>::Value;

// Template Min Of Value Pack ----------------------------------------------------------

template<typename PackT>
struct TMinPack;

template<auto... Values>
struct TMinPack<TValuePack<Values...>>
{
	static constexpr auto Value = TMin<Values...>::Value;
};

template<typename PackT>
inline constexpr auto TMinPackV = TMinPack<PackT>::Value;

// Type Size Helper ----------------------------------------------------------

template<typename... Types>
struct TTypeSize;

template<>
struct TTypeSize<> { };

template<typename T>
struct TTypeSize<T>
{
	static constexpr size_t Max = sizeof(T);
	static constexpr size_t Min = sizeof(T);
};

template<typename Type, typename... Rest>
struct TTypeSize<Type, Rest...>
{
	using SizeRest = TTypeSize<Rest...>;
	
	static constexpr size_t Max = TMax<sizeof(Type), SizeRest::Max>::Value;
	static constexpr size_t Min = TMin<sizeof(Type), SizeRest::Min>::Value;
};

// TTypeSize specialization for TTypePack -----------------------------------------

template<>
struct TTypeSize<TTypePack<>> : public TTypeSize<> { };

template<typename... Types>
struct TTypeSize<TTypePack<Types...>> : public TTypeSize<Types...> { };

// -------------------------------------------------------------------------------------
// - Has Function Test
// -------------------------------------------------------------------------------------

#define DECLARE_THASFUNCTION(funcname) \
namespace _THelpers \
{ \
	template<typename ClassT> \
	struct _THas##funcname \
	{ \
		template<typename T> static constexpr TBool<true>  Test(decltype(&T::##funcname)); \
		template<typename T> static constexpr TBool<false> Test(...); \
	public: \
		static constexpr bool Value = decltype(Test<ClassT>(0))::value; \
	}; \
} \
template<typename ClassT> \
static constexpr bool THas##funcname = _THelpers::_THas##funcname<ClassT>::Value
