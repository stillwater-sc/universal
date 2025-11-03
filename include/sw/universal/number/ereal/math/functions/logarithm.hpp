#pragma once
// logarithm.hpp: logarithm functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// log: natural logarithm (base e)
	// Phase 4a: Implementation using range reduction + Taylor series
	// Strategy:
	//   1. Range reduction: use frexp to extract mantissa and exponent
	//      x = m * 2^e where 0.5 <= m < 1
	//   2. log(x) = log(m) + e*ln(2)
	//   3. For log(m), use transformation for better convergence:
	//      m = (1+u)/(1-u) => log(m) = log((1+u)/(1-u)) = 2(u + u³/3 + u⁵/5 + ...)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// Handle special cases
		if (x.iszero()) {
			// log(0) = -inf, but ereal doesn't support inf, return large negative
			return Real(-1.0e308);
		}
		if (x.isneg()) {
			// log(negative) = NaN
			return Real(std::numeric_limits<double>::quiet_NaN());
		}
		if (x.isone()) return Real(0.0);

		// Range reduction using frexp: x = m * 2^e where 0.5 <= m < 1
		int exponent;
		Real mantissa = frexp(x, &exponent);

		// log(x) = log(m) + e*ln(2)
		// ln(2) ≈ 0.693147180559945309417232121458176568075500134360255254120680009
		Real ln2(0.6931471805599453);

		// For log(m) where 0.5 <= m < 1, we want to use a series
		// Transform: m = (1+u)/(1-u), solve for u: u = (m-1)/(m+1)
		// Then: log(m) = 2(u + u³/3 + u⁵/5 + u⁷/7 + ...)
		// This converges faster than log(1+z) series

		Real one(1.0);
		Real u = (mantissa - one) / (mantissa + one);

		// Series: log(m) = 2(u + u³/3 + u⁵/5 + ...)
		Real u_squared = u * u;
		Real term = u;
		Real result = term;

		double epsilon = 1.0e-17;

		for (int n = 1; n < 100; ++n) {
			// Next term: u^(2n+1) / (2n+1)
			term = term * u_squared;
			Real denominator = Real(double(2 * n + 1));
			Real series_term = term / denominator;
			result = result + series_term;

			// Check convergence
			double term_mag = std::abs(double(series_term));
			if (term_mag < epsilon) break;
		}

		// Multiply by 2 for the series
		Real two(2.0);
		result = result * two;

		// Add e*ln(2)
		if (exponent != 0) {
			Real exp_term = Real(double(exponent)) * ln2;
			result = result + exp_term;
		}

		return result;
	}

	// log2: binary logarithm (base 2)
	// Phase 4a: implement using log(x) / log(2)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log2(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;
		// ln(2) ≈ 0.693147180559945309417232121458176568075500134360255254120680009
		Real ln2(0.6931471805599453);
		return log(x) / ln2;
	}

	// log10: common logarithm (base 10)
	// Phase 4a: implement using log(x) / log(10)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log10(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;
		// ln(10) ≈ 2.302585092994045684017991454684364207601101488628772976033327900
		Real ln10(2.302585092994045684);
		return log(x) / ln10;
	}

	// log1p: compute log(1 + x) accurately for small x
	// Phase 4a: implement using Taylor series (avoids cancellation for small x)
	// For small x: log(1+x) = x - x²/2 + x³/3 - x⁴/4 + ...
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log1p(const ereal<maxlimbs>& x) {
		using Real = ereal<maxlimbs>;

		// For small x, use Taylor series directly
		// This avoids catastrophic cancellation in log(1+x)
		Real threshold(0.1);
		Real neg_threshold(-0.1);

		if (x < threshold && x > neg_threshold) {
			// Taylor series: log(1+x) = x - x²/2 + x³/3 - x⁴/4 + ...
			Real result = x;
			Real term = x;
			Real neg_x = -x;

			double epsilon = 1.0e-17;

			for (int n = 2; n < 100; ++n) {
				// Alternating series: term_n = -term_{n-1} * x / n
				// NOTE: Use double literal to avoid ereal(int) constructor bug
				term = term * neg_x / Real(double(n));
				result = result + term;

				double term_mag = std::abs(double(term));
				if (term_mag < epsilon) break;
			}

			return result;
		}
		else {
			// For larger x, use log(1+x)
			Real one(1.0);
			return log(one + x);
		}
	}

}} // namespace sw::universal
