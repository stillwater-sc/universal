#pragma once
// cross_type_traits.hpp: type traits and C++20 concepts for cross-type conversions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This header provides the foundational traits and concepts for enabling
// automatic conversions between different Universal number types without
// creating module cycles. It uses interface-based detection rather than
// type identity, so number types do not need to know about each other.

#include <type_traits>
#include <concepts>
#include <limits>
#include <string_view>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// C++20 Concept: UniversalNumber
//
// Detects any Universal number type by its common interface.
// All Universal number types provide:
//   - explicit operator double() for value extraction
//   - iszero(), sign() for state queries
//   - default constructibility
//
// This concept deliberately avoids naming specific types, breaking
// the module cycle described in issue #197.

template<typename T>
concept UniversalNumber = requires(const T& t) {
	{ static_cast<double>(t) } -> std::convertible_to<double>;
	{ t.iszero() } -> std::convertible_to<bool>;
	{ t.sign() } -> std::convertible_to<bool>;
	{ T{} };
} && !std::is_arithmetic_v<std::remove_cvref_t<T>>;

////////////////////////////////////////////////////////////////////
// C++20 Concept: UniversalFloatingPoint
//
// Detects Universal number types that have floating-point semantics
// (NaN, Inf support). These types support the blocktriple conversion
// path for higher-precision cross-type conversion.

template<typename T>
concept UniversalFloatingPoint = UniversalNumber<T> && requires(const T& t) {
	{ t.isnan() } -> std::convertible_to<bool>;
	{ t.isinf() } -> std::convertible_to<bool>;
	{ t.scale() } -> std::convertible_to<int>;
};

////////////////////////////////////////////////////////////////////
// SFINAE helpers for pre-C++20 usage

template<typename T, typename = void>
struct is_universal_arithmetic : std::false_type {};

template<typename T>
struct is_universal_arithmetic<T,
	std::void_t<
		decltype(static_cast<double>(std::declval<const T&>())),
		decltype(std::declval<const T&>().iszero()),
		decltype(std::declval<const T&>().sign()),
		decltype(T{})
	>> : std::bool_constant<!std::is_arithmetic_v<std::remove_cvref_t<T>>> {};

template<typename T>
inline constexpr bool is_universal_arithmetic_v = is_universal_arithmetic<T>::value;

////////////////////////////////////////////////////////////////////
// Precision traits for promotion rules
//
// precision_bits<T> returns the effective precision (in bits) of a
// Universal number type. Used by mixed-type operators to determine
// which type to promote to. Higher precision wins.

template<typename T, typename = void>
struct precision_bits : std::integral_constant<unsigned, 0> {};

// For types with fbits (cfloat, posit, areal)
template<typename T>
struct precision_bits<T, std::void_t<decltype(T::fbits)>>
	: std::integral_constant<unsigned, T::fbits> {};

// Fallback: use sizeof * 8 as a rough precision estimate
template<typename T>
inline constexpr unsigned precision_bits_v =
	(precision_bits<T>::value > 0) ? precision_bits<T>::value : (sizeof(T) * 8);

////////////////////////////////////////////////////////////////////
// Promotion trait: given two Universal types, pick the one that covers both (#2059)
//
// A mixed operation must not overflow where either operand's type would not, and should keep as
// many significand bits as it can. Each type is ranked by a key, compared lexicographically:
//
//   1. dynamic range at the top:    numeric_limits<T>::max_exponent, larger wins
//   2. precision:                   numeric_limits<T>::digits (significand bits), larger wins
//   3. dynamic range at the bottom: numeric_limits<T>::min_exponent, smaller wins
//   4. storage:                     sizeof(T), larger wins
//   5. the type's name, so that equal keys still give one answer
//
// When one type covers the other in range and precision, it wins. When neither does (cfloat<16,8>
// against cfloat<16,5>: more range, less precision), range decides, because a sum past the
// narrow type's maxpos is lost entirely while the extra significand bits round away gracefully.
// The key does not depend on operand order: promote_t<A, B> and promote_t<B, A> are the same type.
// A type without a numeric_limits specialization is ranked by precision_bits_v alone, below any
// type that has one.

namespace detail {

	template<typename T>
	constexpr bool has_limits_v = std::numeric_limits<T>::is_specialized;

	template<typename T>
	constexpr long promotion_max_exponent() {
		if constexpr (has_limits_v<T>) return static_cast<long>(std::numeric_limits<T>::max_exponent);
		else return std::numeric_limits<long>::min();   // no limits: below every type that has them
	}

	template<typename T>
	constexpr long promotion_digits() {
		if constexpr (has_limits_v<T>) return static_cast<long>(std::numeric_limits<T>::digits);
		else return static_cast<long>(precision_bits_v<T>) + 1;
	}

	template<typename T>
	constexpr long promotion_min_exponent() {
		if constexpr (has_limits_v<T>) return static_cast<long>(std::numeric_limits<T>::min_exponent);
		else return 0;
	}

	// the compiler's spelling of the type: unique per type, so it orders any two distinct types
	template<typename T>
	constexpr std::string_view promotion_type_name() {
#if defined(_MSC_VER) && !defined(__clang__)
		return __FUNCSIG__;
#else
		return __PRETTY_FUNCTION__;
#endif
	}

	// true when T1 ranks above T2; for distinct types exactly one of (T1, T2), (T2, T1) is true
	template<typename T1, typename T2>
	constexpr bool promotion_ranks_above() {
		constexpr long r1 = promotion_max_exponent<T1>(), r2 = promotion_max_exponent<T2>();
		constexpr long p1 = promotion_digits<T1>(),       p2 = promotion_digits<T2>();
		constexpr long m1 = promotion_min_exponent<T1>(), m2 = promotion_min_exponent<T2>();
		return (r1 != r2) ? (r1 > r2)
		     : (p1 != p2) ? (p1 > p2)
		     : (m1 != m2) ? (m1 < m2)
		     : (sizeof(T1) != sizeof(T2)) ? (sizeof(T1) > sizeof(T2))
		     : (promotion_type_name<T1>() < promotion_type_name<T2>());
	}

}  // namespace detail

template<typename T1, typename T2>
using promote_t = std::conditional_t<
	std::is_same_v<T1, T2> || detail::promotion_ranks_above<T1, T2>(), T1, T2>;

}} // namespace sw::universal
