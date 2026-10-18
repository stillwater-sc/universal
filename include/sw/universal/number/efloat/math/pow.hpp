// pow.hpp: power functions for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/efloat/math/exponent.hpp>
#include <universal/number/efloat/math/logarithm.hpp>
#include <universal/number/efloat/math/classify.hpp>
#include <universal/number/efloat/math/truncate.hpp>

namespace sw { namespace universal {

namespace efloat_detail {

	// exp(y log x) for x > 0, with log x and the product carried at the working precision and
	// rounded once to the result's (#1712): an error e in log x becomes an error y e in the
	// exponent, so at the result's precision x^y would lose log2|y log x| bits
	template<unsigned nlimbs>
	inline efloat<nlimbs> exp_of_product_log(const efloat<nlimbs>& y, const efloat<nlimbs>& x) {
		const unsigned precision = std::max(result_precision(x), y.get_precision());
		const unsigned working   = precision + exp_log_guard_bits;
		efloat<nlimbs> xw(x), yw(y);
		xw.set_precision(working);
		yw.set_precision(working);
		efloat<nlimbs> result = exp(yw * log(xw));
		result.set_precision(precision);
		return result;
	}

}  // namespace efloat_detail

// Fast integer power using binary exponentiation, fully safe from INT_MIN overflow
template<unsigned nlimbs>
constexpr efloat<nlimbs> integer_power(efloat<nlimbs> base, int exponent) {
	if (exponent == 0) return efloat<nlimbs>(1.0);
	uint64_t exp_mag = 0;
	if (exponent < 0) {
		base = efloat<nlimbs>(1.0) / base;
		// Safe negation to prevent INT_MIN overflow undefined behavior
		exp_mag = 0ULL - static_cast<uint64_t>(exponent);
	} else {
		exp_mag = static_cast<uint64_t>(exponent);
	}

	efloat<nlimbs> power(1.0);
	while (exp_mag > 1) {
		if (exp_mag & 0x1) {
			power = base * power;
			base *= base;
			exp_mag = (exp_mag - 1) / 2;
		} else {
			base *= base;
			exp_mag /= 2;
		}
	}
	return base * power;
}

// pow(x, int_y): fast integer power overload
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, int y) {
	return integer_power(x, y);
}

// pow(x, y): arbitrary precision power function
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	// Standard IEEE-754 Boundary rules:
	// pow(x, +/-0) is 1 for any x (including NaN)
	if (y.iszero()) {
		return efloat<nlimbs>(1.0);
	}
	// pow(1, y) is 1 for any y (including NaN)
	if (x == 1.0) {
		return efloat<nlimbs>(1.0);
	}
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	if (x.iszero()) {
		if (y.sign() == -1) {
			efloat<nlimbs> inf; inf.setinf(false);
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
			}
			return inf;
		}
		return efloat<nlimbs>(0.0);
	}

	// If x is negative, the exponent y must be an integer, otherwise the result is complex (NaN in real space)
	if (x.sign() == -1) {
		efloat<nlimbs> y_floor = floor(y);
		if (y != y_floor) {
			efloat<nlimbs> nan; nan.setnan();
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
			}
			return nan;
		}
		// y is an integer! Perform an efloat-native parity check on y without long-long narrowing (CodeRabbit feedback)
		bool is_odd = false;
		if (y.scale() >= 0) {
			size_t full_limbs = static_cast<size_t>(y.scale() / 32);
			unsigned bit_idx = static_cast<unsigned>(y.scale() % 32);
			is_odd = (y.bits()[full_limbs] & (1u << (31u - bit_idx))) != 0;
		}

		// Compute using abs(x)
		efloat<nlimbs> abs_x(x);
		abs_x.setsign(false);
		efloat<nlimbs> res = efloat_detail::exp_of_product_log(y, abs_x);
		if (is_odd) {
			res.setsign(true);
		}
		return res;
	}

	// Normal positive base: x^y = exp(y * log(x))
	return efloat_detail::exp_of_product_log(y, x);
}

}} // namespace sw::universal
