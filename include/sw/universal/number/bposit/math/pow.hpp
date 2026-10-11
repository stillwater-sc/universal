#pragma once
// pow.hpp: power functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
//
// pow(x, y) with x != 0 is finite and nonzero, so it saturates.  pow(0, y) is 0 for
// y > 0, 1 for y = 0, and a pole (NaR) for y < 0.  A negative base with a non-integer
// exponent is a domain error, NaR.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

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
	// the sign of a nonzero power: negative only for a negative base and an odd integer exponent
	const bool negative = (x < 0.0) && (std::fmod(y, 2.0) != 0.0);
	return nonzero<BPosit>(r, negative);
}
} // namespace bposit_shim

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> pow(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) { return bposit_shim::pow<bposit<nbits, rs, es, bt>>(double(x), double(y)); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> pow(const bposit<nbits, rs, es, bt>& x, int y) { return bposit_shim::pow<bposit<nbits, rs, es, bt>>(double(x), static_cast<double>(y)); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> pow(const bposit<nbits, rs, es, bt>& x, double y) { return bposit_shim::pow<bposit<nbits, rs, es, bt>>(double(x), y); }

}} // namespace sw::universal
