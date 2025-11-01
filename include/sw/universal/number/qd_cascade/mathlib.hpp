#pragma once
// mathlib.hpp: definition of mathematical functions for qd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/qd_cascade/math/functions/numerics.hpp>

// Phase 1: Low-complexity stub functions (completed)
#include <universal/number/qd_cascade/math/functions/error_and_gamma.hpp>
#include <universal/number/qd_cascade/math/functions/fractional.hpp>
#include <universal/number/qd_cascade/math/functions/hypot.hpp>
#include <universal/number/qd_cascade/math/functions/minmax.hpp>
#include <universal/number/qd_cascade/math/functions/truncate.hpp>

// Phase 2: Medium-complexity functions (completed)
#include <universal/number/qd_cascade/math/functions/cbrt.hpp>
#include <universal/number/qd_cascade/math/functions/hyperbolic.hpp>
#include <universal/number/qd_cascade/math/functions/exponent.hpp>
#include <universal/number/qd_cascade/math/functions/logarithm.hpp>
#include <universal/number/qd_cascade/math/functions/pow.hpp>

// Phase 3: High-complexity functions (completed)
#include <universal/number/qd_cascade/math/functions/sqrt.hpp>
#include <universal/number/qd_cascade/math/functions/trigonometry.hpp>
// Note: classify (fpclassify, isinf, isnan, isfinite, isnormal) in attributes.hpp
// Note: next (nextafter, ulp) in attributes.hpp or numerics.hpp

namespace sw { namespace universal {

	// pown returns x raised to the integer power n
	inline qd_cascade pown(qd_cascade x, int n) {
		// Delegate to floatcascade base class implementation
		floatcascade<4> fc = x;  // Convert to floatcascade
		floatcascade<4> result = sw::universal::pown(fc, n);
		return qd_cascade(result);
	}

	// Note: exp(), expm1(), exp2(), exp10() are defined in math/functions/exponent.hpp
	// Note: pow(), npwr() are defined in math/functions/pow.hpp
	// Note: sinh(), cosh(), tanh(), asinh(), acosh(), atanh() are defined in math/functions/hyperbolic.hpp
	// TODO: Phase 3 - logarithm, sqrt, trigonometry functions

	// Note: floor(), ceil(), trunc(), round() are defined in math/functions/truncate.hpp
	// Note: fmod(), remainder() are defined in math/functions/fractional.hpp
	// Note: min(), max() are defined in math/functions/minmax.hpp
	// Note: hypot() is defined in math/functions/hypot.hpp
	// Note: erf(), erfc() are defined in math/functions/error_and_gamma.hpp
	// Note: copysign, frexp, ldexp are defined in math/functions/numerics.hpp

	// Additional TODO items from classic qd:
	// - modf (extract integer and fractional parts) - in fractional.hpp when ready
	// - nextafter, nexttoward (adjacent representable value) - in next.hpp when ready
	// - fdim (positive difference)

}} // namespace sw::universal
