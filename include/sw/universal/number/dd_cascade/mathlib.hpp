#pragma once
// mathlib.hpp: definition of mathematical functions for dd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// TODO: Port mathematical functions from classic dd implementation
	// The following are placeholder implementations using double approximations
	// They should be replaced with high-precision dd_cascade implementations

	// exp returns the exponential e^x
	inline dd_cascade exp(dd_cascade x) {
		// TODO: Port accurate exp from classic dd
		return dd_cascade(std::exp(x.high()));
	}

	// log returns the natural logarithm ln(x)
	inline dd_cascade log(dd_cascade x) {
		// TODO: Port accurate log from classic dd
		return dd_cascade(std::log(x.high()));
	}

	// log10 returns the base-10 logarithm
	inline dd_cascade log10(dd_cascade x) {
		// TODO: Port accurate log10 from classic dd
		return dd_cascade(std::log10(x.high()));
	}

	// log2 returns the base-2 logarithm
	inline dd_cascade log2(dd_cascade x) {
		// TODO: Port accurate log2 from classic dd
		return dd_cascade(std::log2(x.high()));
	}

	// sin returns the sine of x (x in radians)
	inline dd_cascade sin(dd_cascade x) {
		// TODO: Port accurate sin from classic dd
		return dd_cascade(std::sin(x.high()));
	}

	// cos returns the cosine of x (x in radians)
	inline dd_cascade cos(dd_cascade x) {
		// TODO: Port accurate cos from classic dd
		return dd_cascade(std::cos(x.high()));
	}

	// tan returns the tangent of x (x in radians)
	inline dd_cascade tan(dd_cascade x) {
		// TODO: Port accurate tan from classic dd
		return dd_cascade(std::tan(x.high()));
	}

	// asin returns the arc sine of x
	inline dd_cascade asin(dd_cascade x) {
		// TODO: Port accurate asin from classic dd
		return dd_cascade(std::asin(x.high()));
	}

	// acos returns the arc cosine of x
	inline dd_cascade acos(dd_cascade x) {
		// TODO: Port accurate acos from classic dd
		return dd_cascade(std::acos(x.high()));
	}

	// atan returns the arc tangent of x
	inline dd_cascade atan(dd_cascade x) {
		// TODO: Port accurate atan from classic dd
		return dd_cascade(std::atan(x.high()));
	}

	// atan2 returns the arc tangent of y/x using the signs to determine the quadrant
	inline dd_cascade atan2(dd_cascade y, dd_cascade x) {
		// TODO: Port accurate atan2 from classic dd
		return dd_cascade(std::atan2(y.high(), x.high()));
	}

	// sinh returns the hyperbolic sine of x
	inline dd_cascade sinh(dd_cascade x) {
		// TODO: Port accurate sinh from classic dd
		return dd_cascade(std::sinh(x.high()));
	}

	// cosh returns the hyperbolic cosine of x
	inline dd_cascade cosh(dd_cascade x) {
		// TODO: Port accurate cosh from classic dd
		return dd_cascade(std::cosh(x.high()));
	}

	// tanh returns the hyperbolic tangent of x
	inline dd_cascade tanh(dd_cascade x) {
		// TODO: Port accurate tanh from classic dd
		return dd_cascade(std::tanh(x.high()));
	}

	// asinh returns the inverse hyperbolic sine of x
	inline dd_cascade asinh(dd_cascade x) {
		// TODO: Port accurate asinh from classic dd
		return dd_cascade(std::asinh(x.high()));
	}

	// acosh returns the inverse hyperbolic cosine of x
	inline dd_cascade acosh(dd_cascade x) {
		// TODO: Port accurate acosh from classic dd
		return dd_cascade(std::acosh(x.high()));
	}

	// atanh returns the inverse hyperbolic tangent of x
	inline dd_cascade atanh(dd_cascade x) {
		// TODO: Port accurate atanh from classic dd
		return dd_cascade(std::atanh(x.high()));
	}

	// pown returns x raised to the integer power n
	inline dd_cascade pown(dd_cascade x, int n) {
		// Delegate to floatcascade base class implementation
		floatcascade<2> fc = x;  // Convert to floatcascade
		floatcascade<2> result = sw::universal::pown(fc, n);
		return dd_cascade(result);
	}

	// floor returns the largest integer value not greater than x
	inline dd_cascade floor(dd_cascade x) {
		dd_cascade result;
		result.high() = std::floor(x.high());
		result.low() = 0.0;
		if (result.high() == x.high()) {
			// high component is already an integer, check low component
			result.low() = std::floor(x.low());
		}
		return result;
	}

	// ceil returns the smallest integer value not less than x
	inline dd_cascade ceil(dd_cascade x) {
		dd_cascade result;
		result.high() = std::ceil(x.high());
		result.low() = 0.0;
		if (result.high() == x.high()) {
			// high component is already an integer, check low component
			result.low() = std::ceil(x.low());
		}
		return result;
	}

	// Additional TODO items from classic dd:
	// - frexp, ldexp (mantissa/exponent manipulation)
	// - modf (extract integer and fractional parts)
	// - fmod, remainder (modular arithmetic)
	// - copysign (copy sign from one value to another)
	// - nextafter, nexttoward (adjacent representable value)
	// - fdim (positive difference)
	// - fmax, fmin (maximum and minimum)

}} // namespace sw::universal
