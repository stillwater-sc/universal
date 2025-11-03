#pragma once
// hyperbolic.hpp: hyperbolic functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// sinh: hyperbolic sine
	// Phase 5: Implementation using (exp(x) - exp(-x)) / 2
	// sinh(x) = (e^x - e^-x) / 2
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

	// cosh: hyperbolic cosine
	// Phase 5: Implementation using (exp(x) + exp(-x)) / 2
	// cosh(x) = (e^x + e^-x) / 2
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

	// tanh: hyperbolic tangent
	// Phase 5: Implementation using sinh/cosh
	// tanh(x) = sinh(x) / cosh(x)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> tanh(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Special cases
		if (x.iszero()) return Real(0.0);

		// tanh(x) = sinh(x) / cosh(x)
		// Alternative: tanh(x) = (exp(2x) - 1) / (exp(2x) + 1)
		// Using the alternative form for better numerical stability
		Real two(2.0);
		Real exp_2x = exp(two * x);
		Real one(1.0);

		return (exp_2x - one) / (exp_2x + one);
	}

	// asinh: inverse hyperbolic sine
	// Phase 5: Implementation using log(x + sqrt(x^2 + 1))
	// asinh(x) = log(x + sqrt(x² + 1))
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

	// acosh: inverse hyperbolic cosine
	// Phase 5: Implementation using log(x + sqrt(x^2 - 1))
	// acosh(x) = log(x + sqrt(x² - 1)) for x >= 1
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

	// atanh: inverse hyperbolic tangent
	// Phase 5: Implementation using 0.5 * log((1 + x) / (1 - x))
	// atanh(x) = 0.5 * log((1 + x) / (1 - x)) for |x| < 1
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
