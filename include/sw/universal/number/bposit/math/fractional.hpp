#pragma once
// fractional.hpp: fractional functions (fmod, remainder, frac) for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fmod and remainder are exact in double, and frac is x - trunc(x), also exact, so each
// result is rounded once.  The remainder of a division by zero is NaR.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

// x - n y, n = trunc(x / y)
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> fmod(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) { return bposit<nbits, rs, es, bt>(std::fmod(double(x), double(y))); }

// x - n y, n = x / y rounded to the nearest integer, ties to even
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> remainder(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) { return bposit<nbits, rs, es, bt>(std::remainder(double(x), double(y))); }

// the fractional part, x - trunc(x), with the sign of x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> frac(const bposit<nbits, rs, es, bt>& x) {
	const double d = double(x);
	return bposit<nbits, rs, es, bt>(d - std::trunc(d));
}

}} // namespace sw::universal
