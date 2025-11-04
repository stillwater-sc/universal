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
	// Direct computation using the exponential function:
	//   sinh(x) = (e^x - e^-x) / 2
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
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> sinh(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special cases
		if (x.iszero()) return Real(0.0);

		// sinh(x) = (exp(x) - exp(-x)) / 2
		Real exp_x = exp(x);
		Real exp_neg_x = exp(-x);
		Real two(2.0);

		return (exp_x - exp_neg_x) / two;
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
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> cosh(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

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
	// 1. Uses numerically stable form: tanh(x) = (e^(2x) - 1) / (e^(2x) + 1)
	// 2. Avoids catastrophic cancellation compared to sinh/cosh form
	// 3. Relies on reference exp() implementation for full precision
	//
	// ALGORITHM:
	// ----------
	// Numerically stable computation:
	//   tanh(x) = (e^(2x) - 1) / (e^(2x) + 1)
	//
	// This form is preferred over tanh(x) = sinh(x)/cosh(x) because:
	// - Requires only one exp() call instead of two
	// - Better numerical stability for large |x|
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
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> tanh(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special cases
		if (x.iszero()) return Real(0.0);

		// tanh(x) = (exp(2x) - 1) / (exp(2x) + 1)
		// Using this form for better numerical stability
		Real two(2.0);
		Real exp_2x = exp(two * x);
		Real one(1.0);

		return (exp_2x - one) / (exp_2x + one);
	}

	// asinh: inverse hyperbolic sine - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision asinh:
	// 1. Uses standard mathematical identity: asinh(x) = log(x + sqrt(x² + 1))
	// 2. Relies on reference log() and sqrt() implementations for full precision
	// 3. Pure ereal arithmetic throughout
	//
	// ALGORITHM:
	// ----------
	// Direct computation using the logarithm:
	//   asinh(x) = log(x + sqrt(x² + 1))
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
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> asinh(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special case
		if (x.iszero()) return Real(0.0);

		// asinh(x) = log(x + sqrt(x² + 1))
		Real x_squared = x * x;
		Real one(1.0);
		Real sqrt_term = sqrt(x_squared + one);

		return log(x + sqrt_term);
	}

	// acosh: inverse hyperbolic cosine - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision acosh:
	// 1. Uses standard mathematical identity: acosh(x) = log(x + sqrt(x² - 1))
	// 2. Proper domain checking (x >= 1)
	// 3. Relies on reference log() and sqrt() implementations for full precision
	//
	// ALGORITHM:
	// ----------
	// Direct computation using the logarithm:
	//   acosh(x) = log(x + sqrt(x² - 1)) for x >= 1
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
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> acosh(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Domain check: x must be >= 1
		Real one(1.0);
		if (x < one) {
			return Real(std::numeric_limits<double>::quiet_NaN());
		}

		// Special case: acosh(1) = 0
		if (x == one) return Real(0.0);

		// acosh(x) = log(x + sqrt(x² - 1))
		Real x_squared = x * x;
		Real sqrt_term = sqrt(x_squared - one);

		return log(x + sqrt_term);
	}

	// atanh: inverse hyperbolic tangent - REFERENCE IMPLEMENTATION
	//
	// This implementation demonstrates best practices for adaptive-precision atanh:
	// 1. Uses standard mathematical identity: atanh(x) = 0.5 * log((1 + x) / (1 - x))
	// 2. Proper domain checking (|x| < 1)
	// 3. Relies on reference log() implementation for full precision
	//
	// ALGORITHM:
	// ----------
	// Direct computation using the logarithm:
	//   atanh(x) = 0.5 * log((1 + x) / (1 - x)) for |x| < 1
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
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> atanh(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special case
		if (x.iszero()) return Real(0.0);

		// Domain check: |x| must be < 1
		Real one(1.0);
		Real abs_x = abs(x);
		if (abs_x >= one) {
			return Real(std::numeric_limits<double>::quiet_NaN());
		}

		// atanh(x) = 0.5 * log((1 + x) / (1 - x))
		Real numerator = one + x;
		Real denominator = one - x;
		Real half(0.5);

		return half * log(numerator / denominator);
	}

}} // namespace sw::universal
