#pragma once
// mathlib.hpp: definition of mathematical functions for dd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


#include <universal/number/dd_cascade/math/functions/numerics.hpp>

#include <universal/number/dd_cascade/math/functions/classify.hpp>
#include <universal/number/dd_cascade/math/functions/error_and_gamma.hpp>
#include <universal/number/dd_cascade/math/functions/exponent.hpp>
#include <universal/number/dd_cascade/math/functions/fractional.hpp>
#include <universal/number/dd_cascade/math/functions/hyperbolic.hpp>
#include <universal/number/dd_cascade/math/functions/hypot.hpp>
#include <universal/number/dd_cascade/math/functions/logarithm.hpp>
#include <universal/number/dd_cascade/math/functions/minmax.hpp>
#include <universal/number/dd_cascade/math/functions/next.hpp>
#include <universal/number/dd_cascade/math/functions/pow.hpp>
#include <universal/number/dd_cascade/math/functions/sqrt.hpp>
#include <universal/number/dd_cascade/math/functions/trigonometry.hpp>
#include <universal/number/dd_cascade/math/functions/truncate.hpp>

#include <universal/number/dd_cascade/math/functions/cbrt.hpp>

namespace sw { namespace universal {

	inline dd_cascade abs(const dd_cascade& a);

	// categorization functions

	inline bool isone(const dd_cascade& a) {
		return a.isone();
	}

	// sign function to be consistent with Universal sign convention
	inline bool sign(const dd_cascade& a) {
		return a.sign() < 0;
	}

	inline bool isneg(const dd_cascade& a) {
		return a.isneg();
	}

	inline bool ispos(const dd_cascade& a) {
		return a.ispos();
	}

	// value functions

	inline dd_cascade fabs(const dd_cascade& a) {
		return abs(a);
	}

	// pown returns x raised to the integer power n
	inline dd_cascade pown(dd_cascade x, int n) {
		// Delegate to floatcascade base class implementation
		floatcascade<2> fc = x;  // Convert to floatcascade
		floatcascade<2> result = sw::universal::pown(fc, n);
		return dd_cascade(result);
	}

	// Note: floor() is defined in math/functions/truncate.hpp

	// Additional TODO items from classic dd:
	// - frexp, ldexp (mantissa/exponent manipulation)
	// - modf (extract integer and fractional parts)
	// - fmod, remainder (modular arithmetic)
	// - copysign (copy sign from one value to another)
	// - nextafter, nexttoward (adjacent representable value)
	// - fdim (positive difference)
	// - fmax, fmin (maximum and minimum)

}} // namespace sw::universal
