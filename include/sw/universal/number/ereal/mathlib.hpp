#pragma once
// mathlib.hpp: definition of mathematical functions for ereal adaptive-precision arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Phase 0: Stub implementations - all functions delegate to std:: via double conversion
// This provides immediate functionality at double precision while we build out
// the high-precision implementations in future phases.

// Numeric operations
#include <universal/number/ereal/math/functions/numerics.hpp>
#include <universal/number/ereal/math/functions/classify.hpp>

// Phase 0: Low-complexity stub functions
#include <universal/number/ereal/math/functions/error_and_gamma.hpp>
#include <universal/number/ereal/math/functions/fractional.hpp>
#include <universal/number/ereal/math/functions/hypot.hpp>
#include <universal/number/ereal/math/functions/minmax.hpp>
#include <universal/number/ereal/math/functions/truncate.hpp>
#include <universal/number/ereal/math/functions/next.hpp>

// Phase 0: Medium-complexity stub functions
#include <universal/number/ereal/math/functions/cbrt.hpp>
#include <universal/number/ereal/math/functions/hyperbolic.hpp>
#include <universal/number/ereal/math/functions/exponent.hpp>
#include <universal/number/ereal/math/functions/logarithm.hpp>
#include <universal/number/ereal/math/functions/pow.hpp>

// Phase 0: High-complexity stub functions
#include <universal/number/ereal/math/functions/sqrt.hpp>
#include <universal/number/ereal/math/functions/trigonometry.hpp>

namespace sw { namespace universal {

	// pown returns x raised to the integer power n
	// Phase 1: Adaptive-precision repeated squaring (no double conversion!)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> pown(const ereal<maxlimbs>& x, int n) {
		using Real = ereal<maxlimbs>;

		// Special cases
		if (n == 0) return Real(1.0);
		if (n == 1) return x;
		if (x.iszero()) {
			if (n < 0) return Real(std::numeric_limits<double>::quiet_NaN());
			return Real(0.0);
		}
		if (x.isone()) return Real(1.0);

		// Handle negative exponents: x^(-n) = 1 / x^n
		if (n < 0) {
			Real result = pown(x, -n);
			return Real(1.0) / result;
		}

		// Positive integer power using repeated squaring
		// This algorithm is O(log n) and maintains full precision
		Real result(1.0);
		Real base = x;
		unsigned int exp = static_cast<unsigned int>(n);

		while (exp > 0) {
			if (exp & 1) {
				result = result * base;  // Uses ereal multiplication, maintains precision
			}
			base = base * base;
			exp >>= 1;
		}

		return result;
	}

	// Note: abs() is already defined in ereal_impl.hpp

	// Note: exp(), expm1(), exp2(), exp10() are defined in math/functions/exponent.hpp
	// Note: pow(), pown() are defined above and in math/functions/pow.hpp
	// Note: sinh(), cosh(), tanh(), asinh(), acosh(), atanh() are defined in math/functions/hyperbolic.hpp
	// Note: log(), log2(), log10(), log1p() are defined in math/functions/logarithm.hpp
	// Note: sqrt(), cbrt() are defined in math/functions/sqrt.hpp and cbrt.hpp
	// Note: sin(), cos(), tan(), asin(), acos(), atan(), atan2() are defined in math/functions/trigonometry.hpp
	// Note: floor(), ceil(), trunc(), round() are defined in math/functions/truncate.hpp
	// Note: fmod(), remainder() are defined in math/functions/fractional.hpp
	// Note: min(), max() are defined in math/functions/minmax.hpp
	// Note: hypot() is defined in math/functions/hypot.hpp
	// Note: erf(), erfc(), tgamma(), lgamma() are defined in math/functions/error_and_gamma.hpp
	// Note: copysign(), frexp(), ldexp() are defined in math/functions/numerics.hpp
	// Note: fpclassify(), isnan(), isinf(), isfinite(), isnormal(), signbit() are defined in math/functions/classify.hpp
	// Note: nextafter(), nexttoward() are defined in math/functions/next.hpp

	// Future TODO items for high-precision implementation:
	// Phase 1: Refine simple functions using expansion arithmetic
	//   - truncate, minmax, fractional, hypot, error_and_gamma
	//   - numerics (frexp, ldexp especially important for scaling)
	//   - classification functions
	// Phase 2: Refine transcendental functions using Taylor series/Newton iteration
	//   - sqrt, cbrt (Newton-Raphson)
	//   - exp, log (Taylor series with argument reduction)
	//   - pow (using exp/log)
	//   - hyperbolic functions (using exp or Taylor series)
	// Phase 3: Refine trigonometric functions
	//   - sin, cos, tan (Taylor series with argument reduction)
	//   - asin, acos, atan (Newton iteration or Taylor series)
	// Phase 4: Add precision control API
	//   - Allow requesting specific precision for operations
	//   - Example: sqrt(x, 200) for 200 bits of precision

}} // namespace sw::universal
