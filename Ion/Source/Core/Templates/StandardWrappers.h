#pragma once

// EnableIf

template<bool Test, typename T = void>
using TEnableIf = std::enable_if<Test, T>;

template<bool Test, typename T = void>
using TEnableIfT = typename std::enable_if_t<Test, T>;

// IsBaseOf

template<typename Base, typename Derived>
using TIsBaseOf = std::is_base_of<Base, Derived>;

template<typename Base, typename Derived>
inline constexpr bool TIsBaseOfV = std::is_base_of_v<Base, Derived>;

// IsIntegral

template<typename T>
using TIsIntegral = std::is_integral<T>;

template<typename T>
inline constexpr bool TIsIntegralV = std::is_integral_v<T>;

// IsFloating

template<typename T>
using TIsFloating = std::is_floating_point<T>;

template<typename T>
inline constexpr bool TIsFloatingV = std::is_floating_point_v<T>;

// IsSame

template<typename T, typename U>
using TIsSame = std::is_same<T, U>;

template<typename T, typename U>
inline constexpr bool TIsSameV = std::is_same_v<T, U>;

// BoolConstant

template<bool val>
using TBool = std::bool_constant<val>;

// IsAnyOf

template<typename T, typename... Types>
inline constexpr bool TIsAnyOfV = std::_Is_any_of_v<T, Types...>;

// Disjunction (OR)

template<typename... T>
using TOr = std::disjunction<T...>;

template<typename... T>
inline constexpr bool TOrV = std::disjunction_v<T...>;

// Conjunction (AND)

template<typename... T>
using TAnd = std::conjunction<T...>;

template<typename... T>
inline constexpr bool TAndV = std::conjunction_v<T...>;

// Negation (NOT)

template<typename T>
using TNot = std::negation<T>;

template<typename T>
inline constexpr bool TNotV = std::negation_v<T>;

// Remove *

template<typename T>
using TRemoveRef = std::remove_reference_t<T>;

template<typename T>
using TRemovePtr = std::remove_pointer_t<T>;

template<typename T>
using TRemoveExtent = std::remove_extent_t<T>;

template<typename T>
using TRemoveAllExtents = std::remove_all_extents_t<T>;

// Numeric Limits

template<typename T>
using TNumericLimits = std::numeric_limits<T>;
