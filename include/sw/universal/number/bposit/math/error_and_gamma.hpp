#pragma once
// error_and_gamma.hpp: error and gamma functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
//
// erfc is positive everywhere, so it saturates to minpos instead of underflowing.
// tgamma has poles at 0 and the negative integers (NaR), is nonzero elsewhere, and
// saturates at both ends.  lgamma, log|tgamma|, has the same poles.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> erf(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::erf(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> erfc(const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar()) return x;
	return bposit_shim::nonzero<bposit<nbits, rs, es, bt>>(std::erfc(double(x)), false);
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tgamma(const bposit<nbits, rs, es, bt>& x) {
	const double d = double(x);
	if (d != d) return x;
	if (d <= 0.0 && d == std::floor(d)) return bposit<nbits, rs, es, bt>(SpecificValue::nar);   // a pole
	// gamma is negative on (-1, 0), (-3, -2), ...: where floor(x) is odd
	const bool negative = (d < 0.0) && (std::fmod(std::floor(d), 2.0) != 0.0);
	return bposit_shim::nonzero<bposit<nbits, rs, es, bt>>(std::tgamma(d), negative);
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> lgamma(const bposit<nbits, rs, es, bt>& x) {
	const double d = double(x);
	if (d != d) return x;
	if (d <= 0.0 && d == std::floor(d)) return bposit<nbits, rs, es, bt>(SpecificValue::nar);   // a pole
	return bposit_shim::finite<bposit<nbits, rs, es, bt>>(std::lgamma(d));
}

}} // namespace sw::universal
