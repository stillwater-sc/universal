#pragma once
// ieee754_numeric.hpp: numeric helper functions for IEEE-754 native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>    // std::nextafter
#include <limits>   // std::numeric_limits

namespace sw { namespace universal {

	////////////////////////////////////////////////////////////////////////
	// numerical helpers

	// return the Unit in the Last Position
	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	inline Real ulp(const Real& a) {
		// the magnitude of the spacing at a: the gap to the next value away from zero, or, at the
		// largest finite value, the gap below it (there is no finite value above). It used to be
		// negative for negative a, inf at max() and NaN at inf (#2059). ulp(inf) is inf, ulp(NaN) NaN.
		if (a != a) return a;
		const Real m = (a < 0) ? -a : a;
		if (m == std::numeric_limits<Real>::infinity()) return m;
		const Real next = std::nextafter(m, std::numeric_limits<Real>::infinity());
		if (next == std::numeric_limits<Real>::infinity()) return m - std::nextafter(m, Real(0));
		return next - m;
	}

	// check if the floating-point number is NaN
    template<typename Real, typename = typename ::std::enable_if<::std::is_floating_point<Real>::value, Real>::type>
    inline bool isnan(const Real& a) {
	    return (std::fpclassify(a) == FP_NAN);
    }

	// check if the floating-point number is infinite
    template<typename Real, typename = typename ::std::enable_if<::std::is_floating_point<Real>::value, Real>::type>
    inline bool isinf(const Real& a) {
	    return (std::fpclassify(a) == FP_INFINITE);
    }

	// check if the floating-point number is zero
	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	inline bool iszero(const Real& a) {
		return (std::fpclassify(a) == FP_ZERO);
	}

	// check if the floating-point number is a denorm
	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	inline bool isdenorm(const Real& a) {
		return (std::fpclassify(a) == FP_SUBNORMAL);
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
