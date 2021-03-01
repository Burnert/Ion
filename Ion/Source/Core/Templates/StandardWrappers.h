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
