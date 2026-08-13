// exponent.hpp: exponent functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/takum/math/takum_log_domain.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> exp(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::exp(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> exp2(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::exp2(double(x)));
}

// Base-10 exponential: 10^x
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> exp10(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::pow(10.0, double(x)));
}

// exp(x) - 1, more accurate near zero
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> expm1(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::expm1(double(x)));
}

// ---------------------------------------------------------------------------
// takum_log
// ---------------------------------------------------------------------------

// exp is the mirror of log: |e^x| == sqrt(e)^(2x), so once x is known as a real
// the result's logarithmic value is just 2x and the encoding is direct.  That
// removes the logarithm the generic conversion would otherwise perform, but the
// argument still has to be evaluated, so this saves one transcendental rather
// than both.  See the note in logarithm.hpp.
template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> exp(const takum_log<nbits, rbits, bt>& x) {
	using TL = takum_log<nbits, rbits, bt>;
	using Codec = typename TL::Codec;
	TL result;
	if (x.isnar()) { result.setnar(); return result; }
	if (x.iszero()) return TL(1.0);                 // e^0 == 1

	// The result's logarithmic value is 2x -- which scales with the VALUE of x, not
	// with its logarithmic value, and so leaves the characteristic range almost
	// immediately.  takum_log<16,3> already reaches 2.3e55, whose double is finite
	// but nowhere near an int64_t.  Saturate BEFORE the conversion: an out-of-range
	// double to integer cast is undefined behaviour, and letting it wrap inverted
	// the answer, returning zero for exp(maxpos) and maxpos for exp(maxneg).
	//
	// The !isfinite guard covers wide rbits, where double(x) itself overflows.
	const double l2 = 2.0 * double(x);
	const double cmax = static_cast<double>(Codec::max_characteristic());
	const double cmin = static_cast<double>(Codec::min_characteristic());
	if (std::isnan(l2))     { result.setnar(); return result; }
	if (l2 >= cmax + 1.0)   { result.maxpos();  return result; }   // includes +inf
	if (l2 <  cmin)         { result.setzero(); return result; }   // includes -inf

	int64_t c = static_cast<int64_t>(l2);
	if (l2 < 0.0 && static_cast<double>(c) != l2) --c;
	double m = l2 - static_cast<double>(c);
	if (m < 0.0) m = 0.0;
	if (m >= 1.0) { m = 0.0; ++c; }
	auto enc = Codec::encode_rounded(c, m);
	if (enc.overflowed())  { result.maxpos();  return result; }
	if (enc.underflowed()) { result.setzero(); return result; }
	result.setbits(enc.magnitude);                  // e^x is always positive
	return result;
}

// 2^x == sqrt(e)^(2 x ln2)
template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> exp2(const takum_log<nbits, rbits, bt>& x) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (x.isnar()) { result.setnar(); return result; }
	if (x.iszero()) return TL(1.0);
	return TL(std::exp2(double(x)));
}

// 10^x
template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> exp10(const takum_log<nbits, rbits, bt>& x) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (x.isnar()) { result.setnar(); return result; }
	if (x.iszero()) return TL(1.0);
	return TL(std::pow(10.0, double(x)));
}

// exp(x) - 1: the subtraction is a linear-domain operation, so no shortcut.
template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> expm1(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::expm1(double(x)));
}

}} // namespace sw::universal
