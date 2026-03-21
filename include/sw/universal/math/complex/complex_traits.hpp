#pragma once
// complex_traits.hpp: type traits and C++20 concepts for complex number support
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <type_traits>
#include <concepts>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// C++20 Concepts for complex number support

/// Concept: Type supports basic arithmetic operations
template<typename T>
concept Arithmetic = requires(T a, T b) {
	{ a + b } -> std::convertible_to<T>;
	{ a - b } -> std::convertible_to<T>;
	{ a * b } -> std::convertible_to<T>;
	{ a / b } -> std::convertible_to<T>;
	{ -a } -> std::convertible_to<T>;
	{ T{} };      // default constructible
};

/// Concept: Type is suitable for complex arithmetic (can convert to/from double)
template<typename T>
concept ComplexCompatible = Arithmetic<T> && requires(T a, double d) {
	{ static_cast<double>(a) } -> std::convertible_to<double>;
	{ T(d) } -> std::convertible_to<T>;
};

////////////////////////////////////////////////////////////////////
// Type traits for detecting Universal number types

/// Primary template: not a Universal number type
template<typename T>
struct is_universal_number : std::false_type {};

/// Helper variable template
template<typename T>
inline constexpr bool is_universal_number_v = is_universal_number<T>::value;

// Note: Specializations for specific number types (posit, cfloat, fixpnt, lns, dd, qd, etc.)
// are provided in the respective number system headers to avoid circular dependencies.
// Each number system should add:
//   template<...params...>
//   struct is_universal_number<number_type<...params...>> : std::true_type {};

////////////////////////////////////////////////////////////////////
// Type traits for detecting complex types
// Note: The specialization for sw::universal::complex is in complex_impl.hpp
// after the complex class is defined.

/// Primary template: not a sw::universal::complex type
template<typename T>
struct is_sw_complex : std::false_type {};

/// Helper variable template
template<typename T>
inline constexpr bool is_sw_complex_v = is_sw_complex<T>::value;

////////////////////////////////////////////////////////////////////
// Type traits for detecting high-precision types requiring native implementations

/// Primary template: not a high-precision type
template<typename T>
struct is_high_precision : std::false_type {};

/// Helper variable template
template<typename T>
inline constexpr bool is_high_precision_v = is_high_precision<T>::value;

// Note: Specializations for dd, qd, and other high-precision types
// are provided in their respective headers.

////////////////////////////////////////////////////////////////////
// SFINAE helpers

/// Enable if type is ComplexCompatible
template<typename T, typename Type = void>
using enable_if_complex_compatible = std::enable_if_t<ComplexCompatible<T>, Type>;

/// Enable if type is sw::universal::complex
template<typename T, typename Type = void>
using enable_if_sw_complex = std::enable_if_t<is_sw_complex_v<T>, Type>;

}} // namespace sw::universal
