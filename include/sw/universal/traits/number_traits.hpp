#pragma once
//  number_traits.hpp : number system traits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/traits/metaprogramming.hpp>

namespace sw { 

namespace universal {
	// forward reference
	template<typename ScalarType> struct number_traits;
}

namespace internal {

	// default implementation of digits10(), based on numeric_limits if specialized,
	// 0 for integer types, and log10(epsilon()) otherwise.
	template< typename ScalarType,
		bool use_numeric_limits = std::numeric_limits<ScalarType>::is_specialized,
		bool is_integer = sw::universal::number_traits<ScalarType>::is_integer>
		struct default_digits10_impl
	{
		static int run() { return std::numeric_limits<ScalarType>::digits10; }
	};

	template<typename ScalarType>
	struct default_digits10_impl<ScalarType, false, false> // Floating point
	{
		static int run() {
			return int(ceil(-log10(sw::universal::number_traits<ScalarType>::epsilon())));
		}
	};

	template<typename ScalarType>
	struct default_digits10_impl<ScalarType, false, true> // Integer
	{
		static int run() { return 0; }
	};

} // namespace internal

namespace universal {

	template<typename ScalarType>
	struct generic_number_traits {
		enum : std::uint8_t {
			is_integer = std::numeric_limits<ScalarType>::is_integer,
			is_signed = std::numeric_limits<ScalarType>::is_signed,
			is_complex = 0,
			needs_init = sw::internal::is_arithmetic<ScalarType>::value ? 0 : 1
		};
		static ScalarType epsilon() {
			return numext::numeric_limits<ScalarType>::epsilon();
		}
		static int digits10() {
			return sw::internal::default_digits10_impl<ScalarType>::run();
		}

		static ScalarType max() {
			return (numext::numeric_limits<ScalarType>::max)();
		}

		static ScalarType min() {
			return (numext::numeric_limits<ScalarType>::min)();
		}

		static ScalarType infinity() {
			return numext::numeric_limits<ScalarType>::infinity();
		}

		static ScalarType quiet_NaN() {
			return numext::numeric_limits<ScalarType>::quiet_NaN();
		}
	};

	template<typename ScalarType>
	struct number_traits : public generic_number_traits<ScalarType>
	{};

	template<> struct number_traits<float>
		: generic_number_traits<float>
	{
		//UNUM_DEVICE_FUNC
		static float rough_precision() { return 1e-5f; }
	};

	template<> struct number_traits<double>
		: generic_number_traits<double>
	{
		//UNUM_DEVICE_FUNC
		static double rough_precision() { return 1e-12; }
	};

	template<> struct number_traits<long double>
		: generic_number_traits<long double>
	{
		//UNUM_DEVICE_FUNC
		static long double rough_precision() { return 1e-15l; }
	};

} // namespace universal


} // namespace sw
