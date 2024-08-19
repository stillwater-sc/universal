#pragma once
// ieee754_numeric.hpp: numeric helper functions for IEEE-754 native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	////////////////////////////////////////////////////////////////////////
	// numerical helpers

	// return the Unit in the Last Position
	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	inline Real ulp(const Real& a) {
		Real next = ((a < 0) ? std::nextafter(a, Real(-INFINITY)) : std::nextafter(a, Real(+INFINITY)));
		return next - a;
	}

	// check if the floating-point number is zero
	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	inline bool iszero(const Real& a) {
		return (std::fpclassify(a) == FP_ZERO);
	}

	// compile time power of 2
	template<typename Real, size_t powerOfTwo,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	inline constexpr Real ipow() {
		Real base = 2.0f;
		Real result = 1.0f;
		size_t exp = powerOfTwo;
		for (;;) {
			if (exp & 1) result *= base;
			exp >>= 1;
			if (!exp) break;
			base *= base;
		}
		return result;
	}

	// run time fast power of 2 with positive exponent
	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	inline constexpr Real ipow(size_t exp) {
		Real base = 2.0f;
		Real result = 1.0f;
		for (;;) {
			if (exp & 1) result *= base;
			exp >>= 1;
			if (!exp) break;
			base *= base;
		}
		return result;
	}

}} // namespace sw::universal
