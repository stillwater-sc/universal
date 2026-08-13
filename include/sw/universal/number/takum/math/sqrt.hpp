// sqrt.hpp: square root function for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <universal/number/takum/math/takum_log_domain.hpp>

namespace sw { namespace universal {

// Reciprocal square root.
template<unsigned nbits, unsigned rbits, typename bt>
inline takum<nbits, rbits, bt> rsqrt(const takum<nbits, rbits, bt>& a) {
	takum<nbits, rbits, bt> v = sqrt(a);
	return takum<nbits, rbits, bt>(1.0) / v;
}

// ---------------------------------------------------------------------------
// takum_log: sqrt is a halving of the logarithmic value
// ---------------------------------------------------------------------------

// |value| == sqrt(e)^l, so the square root is sqrt(e)^(l/2).  Halving l is exact
// in the logarithmic domain, which leaves exactly one rounding -- the one the
// target layout forces -- against the three a decode / std::sqrt / encode round
// trip incurs, and it is not capped at a double's 53 significand bits.
//
// Whenever the target layout has a spare fraction bit for the half that an odd
// characteristic contributes, the result is EXACT.
template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> sqrt(const takum_log<nbits, rbits, bt>& a) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (a.isnar()) { result.setnar(); return result; }
	if (a.iszero()) return a;               // sqrt(0) == 0
	if (a.sign())  { result.setnar(); return result; }   // sqrt of a negative is NaR
	return from_log_value<TL>(halve(to_log_value(a)), false);
}

// Reciprocal square root: negate the halved logarithmic value.  Both steps are
// exact, so this does not compound the way 1/sqrt(x) would.
template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> rsqrt(const takum_log<nbits, rbits, bt>& a) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (a.isnar()) { result.setnar(); return result; }
	if (a.iszero()) { result.setnar(); return result; }  // 1/sqrt(0) has no value
	if (a.sign())  { result.setnar(); return result; }
	return from_log_value<TL>(negate(halve(to_log_value(a))), false);
}

// The square, sqrt(e)^(2l).  Doubling l is exact, so squaring rounds at most once.
template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> sqr(const takum_log<nbits, rbits, bt>& a) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (a.isnar()) { result.setnar(); return result; }
	if (a.iszero()) return a;
	return from_log_value<TL>(twice(to_log_value(a)), false);   // a square is positive
}

}} // namespace sw::universal
