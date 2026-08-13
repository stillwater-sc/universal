// logarithm.hpp: logarithm functions for takums
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
takum<nbits, rbits, bt> log(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log(double(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> log2(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log2(double(x)));
}

// Decimal (base-10) logarithm
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> log10(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log10(double(x)));
}

// log(1 + x), more accurate near zero
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> log1p(const takum<nbits, rbits, bt>& x) {
    return takum<nbits, rbits, bt>(std::log1p(double(x)));
}

// ---------------------------------------------------------------------------
// takum_log
// ---------------------------------------------------------------------------

// |x| == sqrt(e)^l == e^(l/2), so the natural logarithm of the magnitude is
// exactly l/2 and no libm call is needed to OBTAIN it.
//
// It is worth being precise about what that saves, because a logarithmic format
// invites overclaiming: extracting the logarithm is free, but expressing the
// answer back as a takum_log needs l' == 2*ln(l/2), which is another logarithm.
// So this replaces two transcendental evaluations with one, and drops a rounding
// -- a real but bounded gain, not the free lunch the representation suggests.
template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> log(const takum_log<nbits, rbits, bt>& x) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (x.isnar() || x.sign()) { result.setnar(); return result; }  // log of a negative
	if (x.iszero())            { result.setnar(); return result; }  // log(0) is -inf, unrepresentable
	return TL(0.5 * to_double(to_log_value(x)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> log2(const takum_log<nbits, rbits, bt>& x) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (x.isnar() || x.sign() || x.iszero()) { result.setnar(); return result; }
	// log2|x| == (l/2) / ln2
	return TL(0.5 * to_double(to_log_value(x)) / 0.6931471805599453);
}

template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> log10(const takum_log<nbits, rbits, bt>& x) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (x.isnar() || x.sign() || x.iszero()) { result.setnar(); return result; }
	// log10|x| == (l/2) / ln10
	return TL(0.5 * to_double(to_log_value(x)) / 2.302585092994046);
}

// log(1 + x) has no logarithmic-domain shortcut: the addition happens in the
// linear domain, which is the operation this representation is worst at.
template<unsigned nbits, unsigned rbits, typename bt>
takum_log<nbits, rbits, bt> log1p(const takum_log<nbits, rbits, bt>& x) {
	return takum_log<nbits, rbits, bt>(std::log1p(double(x)));
}

}} // namespace sw::universal
