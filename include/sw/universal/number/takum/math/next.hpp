// next.hpp: nextafter function for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

namespace sw { namespace universal {

// Return the next representable takum value after x in the direction of target.
// If x == target the function returns target unchanged.
// NaR (Not-a-Real) is the sole exceptional encoding; it sorts outside all finite values.
template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> nextafter(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& target) {
	if (x == target || x.isnar()) return x;
	takum<nbits, rbits, bt> result(x);
	if (target.isnar()) {
		// direction toward NaR: increase magnitude (move away from zero)
		if (x.isneg()) { --result; } else { ++result; }
	}
	else {
		if (x > target) { --result; } else { ++result; }
	}
	return result;
}

}} // namespace sw::universal
