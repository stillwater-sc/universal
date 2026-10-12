#pragma once
// pow.hpp: power functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded for every input (correctly_rounded.hpp), in every configuration whose
// range fits the quad-double engine; the evaluation in double of phase 1 serves the rest.
//
// pow(x, y) with x != 0 is finite and nonzero, so it saturates.  pow(0, y) is 0 for y > 0,
// 1 for y = 0, and a pole (NaR) for y < 0.  A negative base needs an integer exponent, and
// the sign is negative for an odd one; any other negative base is a domain error, NaR.
// Unlike the other functions, pow has exact results and exact midpoints at ordinary
// arguments, pow(9, 0.5) = 3 and the squares of (p + 1) / 2-bit values; the engine's last
// step resolves both.
#include <cmath>
#include <universal/number/bposit/math/qd_kernels.hpp>

namespace sw { namespace universal {

namespace bposit_shim {
template<typename BPosit>
BPosit pow(double x, double y) {
	if (x != x || y != y) return BPosit(SpecificValue::nar);
	if (x == 0.0) {
		if (y > 0.0) return BPosit(0);
		if (y == 0.0) return BPosit(1);
		return BPosit(SpecificValue::nar);
	}
	const double r = std::pow(x, y);
	if (r != r) return BPosit(SpecificValue::nar);
	const bool negative = (x < 0.0) && (std::fmod(y, 2.0) != 0.0);
	return nonzero<BPosit>(r, negative);
}
} // namespace bposit_shim

namespace bposit_math {

// x^y for an exact exponent: y as a quad-double, its parity (-1: not an integer, 0: even,
// 1: odd), and as a double, exact when y_exact is set
template<engine E, typename BP>
BP pow_exact(const BP& x, const qd& yq, int parity, double yd, bool y_exact) {
	if (x.isnar() || yq[0] != yq[0]) return BP(SpecificValue::nar);
	if (yq[0] == 0.0) return BP(1);
	if (x.iszero()) return (yq[0] > 0.0) ? BP(0) : BP(SpecificValue::nar);
	if (x.isneg() && parity < 0) return BP(SpecificValue::nar);
	if (x.isone()) return x;
	if (std::isinf(yd)) {                                             // pow(double) only: the limits
		const BP ax = abs(x);
		if (ax.isone()) return BP(1);
		return ((ax > BP(1)) == (yd > 0.0)) ? BP(SpecificValue::maxpos) : BP(SpecificValue::minpos);
	}
	if constexpr (!qd_engine<BP>) { return bposit_shim::pow<BP>(double(x), yd); }
	else {
		const bool negative = x.isneg() && parity == 1;
		const BP ax = abs(x);
		const double l2 = yd * std::log2(double(ax));                 // log2 of the result, to screen the range
		if (above_range<BP>(l2)) return BP(negative ? SpecificValue::maxneg : SpecificValue::maxpos);
		if (below_range<BP>(l2)) return BP(negative ? SpecificValue::minneg : SpecificValue::minpos);
		if constexpr (E == engine::libm_then_qd && BPOSIT_MATH_FAST_PATH && exact_in_double<BP>) {
			if (y_exact) {
				if (auto r = from_libm<BP>(std::pow(double(x), yd))) return *r;
			}
		}
		const qd v = sw::universal::exp(yq * log_kernel(ax));
		return from_qd<BP>(negative ? -v : v);
	}
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> pow(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) {
	using BP = bposit<nbits, rs, es, bt>;
	return pow_exact<engine::libm_then_qd>(x, to_qd(y), integer_parity(y), double(y), exact_in_double<BP>);
}

} // namespace bposit_math

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> pow(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) { return bposit_math::pow(x, y); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> pow(const bposit<nbits, rs, es, bt>& x, int y) {
	return bposit_math::pow_exact<bposit_math::engine::libm_then_qd>(x, qd(static_cast<double>(y)), (y % 2 != 0) ? 1 : 0, static_cast<double>(y), true);
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> pow(const bposit<nbits, rs, es, bt>& x, double y) {
	if (y != y) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	return bposit_math::pow_exact<bposit_math::engine::libm_then_qd>(x, qd(y), bposit_math::integer_parity(y), y, true);
}

}} // namespace sw::universal
