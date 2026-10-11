#pragma once
// exponent.hpp: exponential functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
//
// e^x, 2^x and 10^x are positive for every real x: an underflow saturates to minpos and
// an overflow to maxpos, never to 0 or NaR.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp(const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar()) return x;
	return bposit_shim::nonzero<bposit<nbits, rs, es, bt>>(std::exp(double(x)), false);
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp2(const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar()) return x;
	return bposit_shim::nonzero<bposit<nbits, rs, es, bt>>(std::exp2(double(x)), false);
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> exp10(const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar()) return x;
	return bposit_shim::nonzero<bposit<nbits, rs, es, bt>>(std::pow(10.0, double(x)), false);
}

// e^x - 1, accurate for small x; it lies in (-1, inf)
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> expm1(const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar()) return x;
	return bposit_shim::finite<bposit<nbits, rs, es, bt>>(std::expm1(double(x)));
}

}} // namespace sw::universal
