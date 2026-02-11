#pragma once
// complex_traits.hpp: number_traits integration for sw::universal::complex
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <type_traits>
#include <universal/traits/integral_constant.hpp>
#include <universal/traits/number_traits.hpp>
#include <universal/math/complex/complex_traits.hpp>

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////
// number_traits specialization for complex<T>

/// number_traits for sw::universal::complex types
template<typename T>
struct number_traits<complex<T>> {
	enum : std::uint8_t {
		is_integer = 0,
		is_signed  = 1,
		is_complex = 1,
		needs_init = 1
	};

	/// Epsilon for complex type (uses underlying type's epsilon)
	static complex<T> epsilon() {
		return complex<T>(number_traits<T>::epsilon(), T{});
	}

	/// Number of decimal digits of precision
	static int digits10() {
		return number_traits<T>::digits10();
	}

	/// Maximum representable value
	static complex<T> max() {
		return complex<T>(number_traits<T>::max(), number_traits<T>::max());
	}

	/// Minimum representable positive value
	static complex<T> min() {
		return complex<T>(number_traits<T>::min(), T{});
	}

	/// Positive infinity representation
	static complex<T> infinity() {
		return complex<T>(number_traits<T>::infinity(), T{});
	}

	/// Quiet NaN representation
	static complex<T> quiet_NaN() {
		return complex<T>(number_traits<T>::quiet_NaN(), T{});
	}

	/// Rough precision for comparisons (used in numerical algorithms)
	static T rough_precision() {
		// Delegate to underlying type if available, otherwise use epsilon
		if constexpr (requires { number_traits<T>::rough_precision(); }) {
			return number_traits<T>::rough_precision();
		} else {
			return number_traits<T>::epsilon();
		}
	}
};

////////////////////////////////////////////////////////////////////
// Type trait for detecting sw::universal::complex types

/// Trait to detect if a type is sw::universal::complex
template<typename _Ty>
struct is_complex_trait : false_type {};

/// Specialization for sw::universal::complex types
template<typename T>
struct is_complex_trait<complex<T>> : true_type {};

/// Helper variable template
template<typename _Ty>
constexpr bool is_complex = is_complex_trait<_Ty>::value;

/// SFINAE helper: enable if type is sw::universal::complex
template<typename _Ty, typename Type = void>
using enable_if_complex = std::enable_if_t<is_complex<_Ty>, Type>;

////////////////////////////////////////////////////////////////////
// Value type extraction for complex types

/// Extract the underlying value type from a complex type
template<typename T>
struct complex_value_type {
	using type = T;  // Default: return T itself
};

/// Specialization for complex types: extract T from complex<T>
template<typename T>
struct complex_value_type<complex<T>> {
	using type = T;
};

/// Helper alias template
template<typename T>
using complex_value_type_t = typename complex_value_type<T>::type;

}} // namespace sw::universal
