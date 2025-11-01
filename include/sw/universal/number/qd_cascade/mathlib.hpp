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

// TODO: Phase 2 & 3 - Port remaining functions from dd_cascade
//#include <universal/number/qd_cascade/math/functions/classify.hpp>
//#include <universal/number/qd_cascade/math/functions/exponent.hpp>
//#include <universal/number/qd_cascade/math/functions/hyperbolic.hpp>
//#include <universal/number/qd_cascade/math/functions/logarithm.hpp>
//#include <universal/number/qd_cascade/math/functions/next.hpp>
//#include <universal/number/qd_cascade/math/functions/pow.hpp>
//#include <universal/number/qd_cascade/math/functions/sqrt.hpp>
//#include <universal/number/qd_cascade/math/functions/trigonometry.hpp>
//#include <universal/number/qd_cascade/math/functions/cbrt.hpp>

namespace sw { namespace universal {

	// pown returns x raised to the integer power n
	inline qd_cascade pown(qd_cascade x, int n) {
		// Delegate to floatcascade base class implementation
		floatcascade<4> fc = x;  // Convert to floatcascade
		floatcascade<4> result = sw::universal::pown(fc, n);
		return qd_cascade(result);
	}

	// TODO: Port mathematical functions from classic qd implementation
	// The following are placeholder implementations using double approximations
	// They should be replaced with high-precision qd_cascade implementations

	// exp returns the exponential e^x
	inline qd_cascade exp(qd_cascade x) {
		// TODO: Port accurate exp from classic qd
		return qd_cascade(std::exp(x[0]));
	}

	// log returns the natural logarithm ln(x)
	inline qd_cascade log(qd_cascade x) {
		// TODO: Port accurate log from classic qd
		return qd_cascade(std::log(x[0]));
	}

	// log10 returns the base-10 logarithm
	inline qd_cascade log10(qd_cascade x) {
		// TODO: Port accurate log10 from classic qd
		return qd_cascade(std::log10(x[0]));
	}

	// log2 returns the base-2 logarithm
	inline qd_cascade log2(qd_cascade x) {
		// TODO: Port accurate log2 from classic qd
		return qd_cascade(std::log2(x[0]));
	}

	// sin returns the sine of x (x in radians)
	inline qd_cascade sin(qd_cascade x) {
		// TODO: Port accurate sin from classic qd
		return qd_cascade(std::sin(x[0]));
	}

	// cos returns the cosine of x (x in radians)
	inline qd_cascade cos(qd_cascade x) {
		// TODO: Port accurate cos from classic qd
		return qd_cascade(std::cos(x[0]));
	}

	// tan returns the tangent of x (x in radians)
	inline qd_cascade tan(qd_cascade x) {
		// TODO: Port accurate tan from classic qd
		return qd_cascade(std::tan(x[0]));
	}

	// asin returns the arc sine of x
	inline qd_cascade asin(qd_cascade x) {
		// TODO: Port accurate asin from classic qd
		return qd_cascade(std::asin(x[0]));
	}

	// acos returns the arc cosine of x
	inline qd_cascade acos(qd_cascade x) {
		// TODO: Port accurate acos from classic qd
		return qd_cascade(std::acos(x[0]));
	}

	// atan returns the arc tangent of x
	inline qd_cascade atan(qd_cascade x) {
		// TODO: Port accurate atan from classic qd
		return qd_cascade(std::atan(x[0]));
	}

	// atan2 returns the arc tangent of y/x using the signs to determine the quadrant
	inline qd_cascade atan2(qd_cascade y, qd_cascade x) {
		// TODO: Port accurate atan2 from classic qd
		return qd_cascade(std::atan2(y[0], x[0]));
	}

	// sinh returns the hyperbolic sine of x
	inline qd_cascade sinh(qd_cascade x) {
		// TODO: Port accurate sinh from classic qd
		return qd_cascade(std::sinh(x[0]));
	}

	// cosh returns the hyperbolic cosine of x
	inline qd_cascade cosh(qd_cascade x) {
		// TODO: Port accurate cosh from classic qd
		return qd_cascade(std::cosh(x[0]));
	}

	// tanh returns the hyperbolic tangent of x
	inline qd_cascade tanh(qd_cascade x) {
		// TODO: Port accurate tanh from classic qd
		return qd_cascade(std::tanh(x[0]));
	}

	// asinh returns the inverse hyperbolic sine of x
	inline qd_cascade asinh(qd_cascade x) {
		// TODO: Port accurate asinh from classic qd
		return qd_cascade(std::asinh(x[0]));
	}

	// acosh returns the inverse hyperbolic cosine of x
	inline qd_cascade acosh(qd_cascade x) {
		// TODO: Port accurate acosh from classic qd
		return qd_cascade(std::acosh(x[0]));
	}

	// atanh returns the inverse hyperbolic tangent of x
	inline qd_cascade atanh(qd_cascade x) {
		// TODO: Port accurate atanh from classic qd
		return qd_cascade(std::atanh(x[0]));
	}

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
