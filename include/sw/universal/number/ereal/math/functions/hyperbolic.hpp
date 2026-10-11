#pragma once
// hyperbolic.hpp: hyperbolic functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// sinh: hyperbolic sine - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision sinh:
	// 1. Uses standard mathematical identity: sinh(x) = (e^x - e^-x) / 2
	// 2. Relies on reference exp() implementation for full precision
	// 3. Pure ereal arithmetic throughout
	//
	// ALGORITHM:
	// ----------
	// From the exponential function:
	//   sinh(x) = (e^x - e^-x) / 2                for |x| >= 1/2
	//   sinh(x) = (e + e / (e + 1)) / 2, e = expm1(x)   for |x| < 1/2, where the difference cancels
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	// [2] MPFR library: https://www.mpfr.org/algorithms.pdf
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to use reference exp() implementation
	//
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> sinh(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// Special cases
		if (x.iszero()) return Real(0.0);

		// sinh is odd: evaluate at a = |x|, so that sinh(-x) is exactly -sinh(x).  Near 0, from
		// expm1: (e + e / (e + 1)) / 2 with e = e^a - 1, where the difference of the two
		// exponentials would cancel (#1668)
		const bool negative = x.isneg();
		const Real a = negative ? -x : x;
		const Real half(0.5);
		Real r;
		if (a < half) {
			const Real e = expm1(a);
			r = (e + e / (e + Real(1.0))) * half;
		}
		else {
			// sinh(a) = (exp(a) - exp(-a)) / 2
			r = (exp(a) - exp(-a)) * half;
		}
		return negative ? -r : r;
	}

	// cosh: hyperbolic cosine - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision cosh:
	// 1. Uses standard mathematical identity: cosh(x) = (e^x + e^-x) / 2
	// 2. Relies on reference exp() implementation for full precision
	// 3. Pure ereal arithmetic throughout
	//
	// ALGORITHM:
	// ----------
	// Direct computation using the exponential function:
	//   cosh(x) = (e^x + e^-x) / 2
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	// [2] MPFR library: https://www.mpfr.org/algorithms.pdf
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to use reference exp() implementation
	//
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> cosh(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// cosh(0) = 1
		if (x.iszero()) return Real(1.0);

		// cosh(x) = (exp(x) + exp(-x)) / 2
		Real exp_x = exp(x);
		Real exp_neg_x = exp(-x);
		Real two(2.0);

		return (exp_x + exp_neg_x) / two;
	}

	// tanh: hyperbolic tangent - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision tanh:
	// 1. One exponential, at a = |x|, and the odd symmetry
	// 2. Avoids catastrophic cancellation near 0 and overflow for large |x|
	// 3. Relies on reference exp() and expm1() implementations for full precision
	//
	// ALGORITHM:
	// ----------
	//   tanh(a) = e / (e + 2), e = expm1(2a)    for a < 1/2
	//   tanh(a) = 1 - 2 / (e^(2a) + 1)          for a >= 1/2, and 1 once e^(2a) overflows
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	// [2] MPFR library: https://www.mpfr.org/algorithms.pdf
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to use reference exp() implementation
	//
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> tanh(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// Special cases
		if (x.iszero()) return Real(0.0);

		// tanh is odd: evaluate at a = |x|.  Near 0, e / (e + 2) with e = expm1(2a), which does not
		// cancel; beyond, 1 - 2 / (e^2a + 1), which saturates to 1 once e^2a overflows.  The form
		// (e^2x - 1) / (e^2x + 1) gave inf / inf = NaN there: tanh(3576) was NaN (#1668).
		const bool negative = x.isneg();
		const Real a = negative ? -x : x;
		const Real one(1.0), two(2.0);
		Real t;
		if (a < Real(0.5)) {
			const Real e = expm1(two * a);
			t = e / (e + two);
		}
		else {
			const Real exp_2a = exp(two * a);
			t = exp_2a.isinf() ? one : one - two / (exp_2a + one);
		}
		return negative ? -t : t;
	}

	// asinh: inverse hyperbolic sine - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision asinh:
	// 1. Uses the identity asinh(x) = log(x + sqrt(x^2 + 1)), in its log1p form
	// 2. Relies on reference log() and sqrt() implementations for full precision
	// 3. Pure ereal arithmetic throughout
	//
	// ALGORITHM:
	// ----------
	// At a = |x|, with the odd symmetry:
	//   asinh(a) = log1p(a + a^2 / (1 + sqrt(1 + a^2)))    which keeps its accuracy near 0
	//   asinh(a) = log(a) + ln 2                          for a > 2^256
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	// [2] MPFR library: https://www.mpfr.org/algorithms.pdf
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to use reference log() implementation
	//
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> asinh(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// Special case
		if (x.iszero()) return Real(0.0);

		// asinh is odd: evaluate at a = |x|, as log1p(a + a^2 / (1 + sqrt(1 + a^2))).  The form
		// log(x + sqrt(x^2 + 1)) takes the logarithm of 1 + x and cancels completely for tiny x:
		// asinh(3e-50) was 0 (#1668).  Beyond 2^256, a^2 would overflow before the correction
		// 1 / (4 a^2) mattered, and asinh(a) = log(a) + ln 2.
		const bool negative = x.isneg();
		const Real a = negative ? -x : x;
		const Real one(1.0);
		Real r;
		if (a > Real(std::ldexp(1.0, 256))) {
			r = log(a) + ereal_ln2<maxlimbs, FpType>();
		}
		else {
			const Real a2 = a * a;
			r = log1p(a + a2 / (one + sqrt(one + a2)));
		}
		return negative ? -r : r;
	}

	// acosh: inverse hyperbolic cosine - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision acosh:
	// 1. Uses the identity acosh(x) = log(x + sqrt(x^2 - 1)), in its log1p form
	// 2. Proper domain checking (x >= 1)
	// 3. Relies on reference log() and sqrt() implementations for full precision
	//
	// ALGORITHM:
	// ----------
	// With t = x - 1, exact:
	//   acosh(x) = log1p(t + sqrt(t (t + 2)))    which keeps its accuracy near 1
	//   acosh(x) = log(x) + ln 2                 for x > 2^256
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	// [2] MPFR library: https://www.mpfr.org/algorithms.pdf
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to use reference log() implementation
	//
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> acosh(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// Domain check: x must be >= 1
		Real one(1.0);
		if (x < one) {
			return std::numeric_limits<Real>::quiet_NaN();
		}

		// Special case: acosh(1) = 0
		if (x == one) return Real(0.0);

		// acosh(x) = log1p(t + sqrt(t (t + 2))) with t = x - 1 exact, which keeps its relative
		// accuracy as x approaches 1.  Beyond 2^256, log(x) + ln 2, as for asinh.
		if (x > Real(std::ldexp(1.0, 256))) return log(x) + ereal_ln2<maxlimbs, FpType>();
		const Real t = x - one;
		return log1p(t + sqrt(t * (t + Real(2.0))));
	}

	// atanh: inverse hyperbolic tangent - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision atanh:
	// 1. Uses the identity atanh(x) = 0.5 * log((1 + x) / (1 - x)), in its log1p form
	// 2. Proper domain checking (|x| < 1)
	// 3. Relies on reference log() implementation for full precision
	//
	// ALGORITHM:
	// ----------
	// At a = |x| < 1, with the odd symmetry:
	//   atanh(a) = 0.5 * log1p(2a / (1 - a))    which keeps its accuracy near 0
	//
	// REFERENCES:
	// -----------
	// [1] Brent, R. P. (1976). "Fast Multiple-Precision Evaluation of Elementary Functions"
	// [2] MPFR library: https://www.mpfr.org/algorithms.pdf
	//
	// HISTORY:
	// --------
	// 2025-01: Refactored to use reference log() implementation
	//
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> atanh(const ereal<maxlimbs, FpType>& x) {
		using Real = ereal<maxlimbs, FpType>;

		// Special case
		if (x.iszero()) return Real(0.0);

		// Domain check: |x| must be < 1
		Real one(1.0);
		Real abs_x = abs(x);
		if (abs_x >= one) {
			return std::numeric_limits<Real>::quiet_NaN();
		}

		// atanh is odd: at a = |x|, log1p(2a / (1 - a)) / 2.  The form log((1 + x) / (1 - x)) / 2
		// cancels completely for tiny x: atanh(1e-30) was 0 (#1668).
		const Real r = log1p(Real(2.0) * abs_x / (one - abs_x)) * Real(0.5);
		return x.isneg() ? -r : r;
	}

}} // namespace sw::universal
