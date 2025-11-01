#pragma once
// mathlib.hpp: definition of mathematical functions for triple-double cascade (td_cascade)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/td_cascade/math/functions/numerics.hpp>

// Phase 1: Low-complexity stub functions (completed)
#include <universal/number/td_cascade/math/functions/error_and_gamma.hpp>
#include <universal/number/td_cascade/math/functions/fractional.hpp>
#include <universal/number/td_cascade/math/functions/hypot.hpp>
#include <universal/number/td_cascade/math/functions/minmax.hpp>
#include <universal/number/td_cascade/math/functions/truncate.hpp>

// TODO: Phase 2 & 3 - Port remaining functions from dd_cascade
// #include <universal/number/td_cascade/math/functions/classify.hpp>
// #include <universal/number/td_cascade/math/functions/exponent.hpp>
// #include <universal/number/td_cascade/math/functions/hyperbolic.hpp>
// #include <universal/number/td_cascade/math/functions/logarithm.hpp>
// #include <universal/number/td_cascade/math/functions/next.hpp>
// #include <universal/number/td_cascade/math/functions/pow.hpp>
// #include <universal/number/td_cascade/math/functions/sqrt.hpp>
// #include <universal/number/td_cascade/math/functions/trigonometry.hpp>
// #include <universal/number/td_cascade/math/functions/cbrt.hpp>

namespace sw { namespace universal {

	// sign function to be consistent with Universal sign convention
	inline bool sign(const td_cascade& a) {
		return a.sign() < 0;
	}

	// pown returns x raised to the integer power n
	inline td_cascade pown(td_cascade x, int n) {
		// Delegate to floatcascade base class implementation
		floatcascade<3> fc = x;  // Convert to floatcascade
		floatcascade<3> result = sw::universal::pown(fc, n);
		return td_cascade(result);
	}

	// TODO: Port mathematical functions from classic td implementation
	// The following are placeholder implementations using double approximations
	// They should be replaced with high-precision td_cascade implementations

	// exp returns the exponential e^x
	inline td_cascade exp(td_cascade x) {
		// TODO: Port accurate exp from classic td
		return td_cascade(std::exp(x[0]));
	}

	// log returns the natural logarithm ln(x)
	inline td_cascade log(td_cascade x) {
		// TODO: Port accurate log from classic td
		return td_cascade(std::log(x[0]));
	}

	// log10 returns the base-10 logarithm
	inline td_cascade log10(td_cascade x) {
		// TODO: Port accurate log10 from classic td
		return td_cascade(std::log10(x[0]));
	}

	// log2 returns the base-2 logarithm
	inline td_cascade log2(td_cascade x) {
		// TODO: Port accurate log2 from classic td
		return td_cascade(std::log2(x[0]));
	}

	// sin returns the sine of x (x in radians)
	inline td_cascade sin(td_cascade x) {
		// TODO: Port accurate sin from classic td
		return td_cascade(std::sin(x[0]));
	}

	// cos returns the cosine of x (x in radians)
	inline td_cascade cos(td_cascade x) {
		// TODO: Port accurate cos from classic td
		return td_cascade(std::cos(x[0]));
	}

	// tan returns the tangent of x (x in radians)
	inline td_cascade tan(td_cascade x) {
		// TODO: Port accurate tan from classic td
		return td_cascade(std::tan(x[0]));
	}

	// asin returns the arc sine of x
	inline td_cascade asin(td_cascade x) {
		// TODO: Port accurate asin from classic td
		return td_cascade(std::asin(x[0]));
	}

	// acos returns the arc cosine of x
	inline td_cascade acos(td_cascade x) {
		// TODO: Port accurate acos from classic td
		return td_cascade(std::acos(x[0]));
	}

	// atan returns the arc tangent of x
	inline td_cascade atan(td_cascade x) {
		// TODO: Port accurate atan from classic td
		return td_cascade(std::atan(x[0]));
	}

	// atan2 returns the arc tangent of y/x using the signs to determine the quadrant
	inline td_cascade atan2(td_cascade y, td_cascade x) {
		// TODO: Port accurate atan2 from classic td
		return td_cascade(std::atan2(y[0], x[0]));
	}

	// sinh returns the hyperbolic sine of x
	inline td_cascade sinh(td_cascade x) {
		// TODO: Port accurate sinh from classic td
		return td_cascade(std::sinh(x[0]));
	}

	// cosh returns the hyperbolic cosine of x
	inline td_cascade cosh(td_cascade x) {
		// TODO: Port accurate cosh from classic td
		return td_cascade(std::cosh(x[0]));
	}

	// tanh returns the hyperbolic tangent of x
	inline td_cascade tanh(td_cascade x) {
		// TODO: Port accurate tanh from classic td
		return td_cascade(std::tanh(x[0]));
	}

	// asinh returns the inverse hyperbolic sine of x
	inline td_cascade asinh(td_cascade x) {
		// TODO: Port accurate asinh from classic td
		return td_cascade(std::asinh(x[0]));
	}

	// acosh returns the inverse hyperbolic cosine of x
	inline td_cascade acosh(td_cascade x) {
		// TODO: Port accurate acosh from classic td
		return td_cascade(std::acosh(x[0]));
	}

	// atanh returns the inverse hyperbolic tangent of x
	inline td_cascade atanh(td_cascade x) {
		// TODO: Port accurate atanh from classic td
		return td_cascade(std::atanh(x[0]));
	}

	// Note: floor(), ceil(), trunc(), round() are defined in math/functions/truncate.hpp
	// Note: fmod(), remainder() are defined in math/functions/fractional.hpp
	// Note: min(), max() are defined in math/functions/minmax.hpp
	// Note: hypot() is defined in math/functions/hypot.hpp
	// Note: erf(), erfc() are defined in math/functions/error_and_gamma.hpp
	// Note: copysign, frexp, ldexp are defined in math/functions/numerics.hpp

	// Additional TODO items from classic td:
	// - modf (extract integer and fractional parts) - in fractional.hpp when ready
	// - nextafter, nexttoward (adjacent representable value) - in next.hpp when ready
	// - fdim (positive difference)

}} // namespace sw::universal
