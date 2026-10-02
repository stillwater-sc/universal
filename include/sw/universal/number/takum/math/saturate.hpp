#pragma once
// saturate.hpp: map a double-evaluated result back into a takum, saturating at the range limits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Several takum and takum_log functions evaluate in a double: TL(std::exp(double(x))).
// From finite arguments, a double result of +/-inf or 0 is usually not the true result
// but the double's own range limit cutting off a finite, nonzero one -- and the takum
// constructor reads those literally: +/-inf becomes NaR (correct for an infinite INPUT,
// wrong here) and 0 becomes zero.  So exp(800) came back as NaR and exp(-800) as zero,
// where takum saturates to maxpos and minpos instead (Hunhold, arXiv:2404.18603,
// Sec. 4.2, and #1620 for conversion and arithmetic).  #1624.
//
// Saturation is only right when the takum's whole range lies inside double's: then a
// double overflow means the true result is beyond maxpos, and an underflow to zero
// means it is below minpos.  That holds for the specified rbits = 3 of both variants
// (2^+/-255 linear, e^+/-127.5 logarithmic).  At rbits = 4 or 5 the takum range dwarfs
// double's, a result that overflows a double can be an ordinary representable takum,
// and saturating would be wrong by orders of magnitude; those keep the literal
// conversion until they get an evaluation that does not go through a double.  The
// range test converts the type's own maxpos and minpos, and runs only on the rare
// inf / zero results, so the common path pays nothing.

#include <cmath>
#include <limits>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {

// r is f(args) evaluated in a double from finite arguments.  nonzero says the true
// result cannot be zero (e^x, cosh, erfc, x^y for x != 0, ...), so a zero r is an
// underflow; pass false where zero is a genuine result.  Callers must not route an
// actual pole through here -- pow(0, -1) is infinite, not large -- since its inf
// would be saturated rather than turned into NaR.
template<typename Takum>
inline Takum saturate_from_double(double r, bool nonzero) {
	const bool overflowed  = (r == std::numeric_limits<double>::infinity()) || (r == -std::numeric_limits<double>::infinity());
	const bool underflowed = (r == 0.0) && nonzero;
	if (overflowed || underflowed) {
		Takum result;
		if (overflowed && std::isfinite(double(Takum(SpecificValue::maxpos)))) {
			return (r > 0.0) ? result.maxpos() : result.maxneg();
		}
		if (underflowed && double(Takum(SpecificValue::minpos)) > 0.0) {
			return std::signbit(r) ? result.minneg() : result.minpos();   // pow(-x, odd) keeps its sign
		}
	}
	return Takum(r);
}

}} // namespace sw::universal
