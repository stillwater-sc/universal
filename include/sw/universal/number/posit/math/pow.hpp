#pragma once
// pow.hpp: power functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded at every nbits (#1855): computed by efloat's correctly rounded pow and rounded once
// into the posit by posit_limbs::evaluate, which applies Ziv's test on the posit side. No native
// floating-point type is involved, so the posit's whole precision and range hold.
//
// The special cases are IEEE-754's (pow(x, 0) = 1, pow(1, y) = 1, pow(0, y > 0) = 0), with NaR for
// NaN: pow(0, y < 0) is a pole and a negative x to a non-integer y has no real value, both NaR, as is
// a NaR operand. A nonzero x has a nonzero power, so past the posit's range the result saturates at
// +-maxpos and +-minpos.
#include <cmath>
#include <universal/number/posit/math/evaluate.hpp>

namespace sw { namespace universal {

	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> pow(posit<nbits,es,bt> x, posit<nbits, es, bt> y) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar() || y.isnar()) return Posit(SpecificValue::nar);
		const auto saturation = x.iszero() ? posit_limbs::Saturation::none : posit_limbs::Saturation::both;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return pow(posit_limbs::argument(x, W), posit_limbs::argument(y, W)); }, saturation);
	}

	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> pow(posit<nbits,es,bt> x, int y) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		const auto saturation = x.iszero() ? posit_limbs::Saturation::none : posit_limbs::Saturation::both;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return pow(posit_limbs::argument(x, W), y); }, saturation);
	}

	// y is a double, converted exactly; a NaN y gives NaR
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> pow(posit<nbits,es,bt> x, double y) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar() || std::isnan(y)) return Posit(SpecificValue::nar);
		const auto saturation = x.iszero() ? posit_limbs::Saturation::none : posit_limbs::Saturation::both;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return pow(posit_limbs::argument(x, W), posit_limbs::efloat_for<nbits>(y)); }, saturation);
	}

#ifndef UNIVERSAL_MATH_INTEGER_POWER_DEFINED
#define UNIVERSAL_MATH_INTEGER_POWER_DEFINED
template<typename Scalar>
Scalar integer_power(Scalar base, int exponent) {
	if (exponent < 0) {
		base = Scalar(1) / base;
		exponent = -exponent;
	}
	if (exponent == 0) return Scalar(1);
	Scalar power = Scalar(1);
	while (exponent > 1) {
		if (exponent & 0x1) {
			power = base * power;
			base *= base;
			exponent = (exponent - 1) / 2;
		}
		else {
			base *= base;
			exponent /= 2;
		}
	}
	return base * power;
}
#endif // UNIVERSAL_MATH_INTEGER_POWER_DEFINED

}} // namespace sw::universal
