#pragma once
// classify.hpp: classification functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A b-posit has one exception value, NaR, and no infinities and no subnormals: every
// encoding other than zero and NaR is a normal real.
#include <cmath>   // FP_NAN, FP_ZERO, FP_NORMAL

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
int fpclassify(const bposit<nbits, rs, es, bt>& x) {
	if (x.isnar()) return FP_NAN;
	if (x.iszero()) return FP_ZERO;
	return FP_NORMAL;
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bool isfinite(const bposit<nbits, rs, es, bt>& x) { return !x.isnar(); }

// there is no infinity: maxpos is where overflow saturates
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bool isinf(const bposit<nbits, rs, es, bt>&) { return false; }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bool isnan(const bposit<nbits, rs, es, bt>& x) { return x.isnar(); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bool isnormal(const bposit<nbits, rs, es, bt>& x) { return !x.iszero() && !x.isnar(); }

// the sign bit; it is set for NaR, as std::signbit reports a NaN's sign bit
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bool signbit(const bposit<nbits, rs, es, bt>& x) { return x.sign(); }

}} // namespace sw::universal
