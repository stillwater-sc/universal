#pragma once
// fractional.hpp: fractional functions (fmod, remainder, frac) for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded for every configuration.  fmod and remainder are computed exactly in
// integers from the encodings' fields, by long division in chunks that fit 64 bits, and
// rounded once: unlike IEEE floats, a b-posit remainder need not be representable, because
// a tapered format has fewer fraction bits far below 1.  frac is x - trunc(x), one rounded
// subtraction of b-posits.  The remainder of a division by zero, or of NaR, is NaR.
#include <bit>
#include <cstdint>
#include <universal/number/bposit/math/correctly_rounded.hpp>
#include <universal/number/bposit/math/truncate.hpp>

namespace sw { namespace universal {

namespace bposit_math {

// x - n y, n = trunc(x / y) (fmod), or n = x / y rounded to nearest, ties to even (remainder)
template<typename B>
B remainder_exact(const B& x, const B& y, bool nearest) {
	if (x.isnar() || y.isnar() || y.iszero()) return B(SpecificValue::nar);
	if (x.iszero()) return x;
	const B ax = abs(x), ay = abs(y);
	if (ax < ay) {
		if (!nearest) return x;
		// n = 0 unless |x| is more than |y| / 2, and a tie (n = 0 or 1) goes to the even n = 0
		const qd twice = sw::universal::mul_pwr2(to_qd(ax), 2.0), qy = to_qd(ay);
		if (twice > qy) return x.isneg() ? x + ay : x - ay;          // one rounding of the exact x -+ |y|
		return x;
	}
	const auto dx = ax.decode_fields(), dy = ay.decode_fields();
	std::uint64_t mx = (std::uint64_t(1) << dx.nf) | dx.frac;
	std::uint64_t my = (std::uint64_t(1) << dy.nf) | dy.frac;
	int ex = dx.scale - static_cast<int>(dx.nf), ey = dy.scale - static_cast<int>(dy.nf);
	const int tx = std::countr_zero(mx), ty = std::countr_zero(my);   // odd significands
	mx >>= tx; ex += tx;
	my >>= ty; ey += ty;
	// in units of 2^unit: X = |x|, Y = |y|, both below 2^62 or X reduced on the fly
	int unit;
	std::uint64_t Y, r;
	bool odd = false;                                                // parity of the quotient
	if (ey > ex) {
		unit = ex;
		Y = my << (ey - ex);                                         // <= |x| / 2^ex < 2^62: fits
		r = mx % Y;
		odd = ((mx / Y) & 1u) != 0u;
	}
	else {
		unit = ey;
		Y = my;
		r = mx % Y;
		std::uint64_t quotient = mx / Y;
		int d = ex - ey;
		const int room = 63 - static_cast<int>(std::bit_width(Y));     // r << room stays below 2^63
		while (d > 0) {
			const int step = d < room ? d : room;
			const std::uint64_t t = r << step;
			quotient = t / Y;
			r = t % Y;
			d -= step;
		}
		odd = (quotient & 1u) != 0u;
	}
	bool negative = x.isneg();
	if (nearest && (2 * r > Y || (2 * r == Y && odd))) {
		r = Y - r;
		negative = !negative;
	}
	if (r == 0u) return B(0);
	const unsigned msb = static_cast<unsigned>(std::bit_width(r)) - 1u;
	const std::uint64_t frac = (msb == 0u) ? 0u : (r & ((std::uint64_t(1) << msb) - 1u));
	B result;
	result.setbits(B::encode(negative, static_cast<int>(msb) + unit, frac, msb, false));
	return result;
}

} // namespace bposit_math

// x - n y, n = trunc(x / y)
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> fmod(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) { return bposit_math::remainder_exact(x, y, false); }

// x - n y, n = x / y rounded to the nearest integer, ties to even
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> remainder(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) { return bposit_math::remainder_exact(x, y, true); }

// the fractional part, x - trunc(x), with the sign of x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> frac(const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar()) return x;
	return x - trunc(x);
}

}} // namespace sw::universal
