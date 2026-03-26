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
// Promotion trait: given two Universal types, pick the wider one

template<typename T1, typename T2>
using promote_t = std::conditional_t<
	(precision_bits_v<T1> >= precision_bits_v<T2>), T1, T2>;

}} // namespace sw::universal
