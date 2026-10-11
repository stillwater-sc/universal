#pragma once
// shim.hpp: rounding a double result into a b-posit, phase 1 of the math library (#1653) for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 evaluates every function in double and rounds the result once into the
// bposit.  The conversion of the argument is exact whenever the bposit's significand
// fits a double (fbits + 1 <= 53) and its range does too (|scale| < 1022), which holds
// for every 16- and 32-bit configuration in use, including the standard <n, 6, 5> and
// the DSP-fitted <16, 4, 2> and <32, 4, 2>.  bposit<64, 6, 5> loses its low bits.
//
// The functions are NOT correctly rounded.  The posit standard requires every function
// to be correctly rounded for every input, because anything less gives up bitwise
// reproducibility.  libm's double result is within about an ulp of the true value, so
// the bposit result is correct except when the true value lies that close to a rounding
// boundary of the bposit; then it can be one encoding off.  Phase 2 of #1653 replaces
// these with correctly rounded implementations.
//
// Two promises of the posit standard survive the double step here:
// - a finite true result never becomes NaR, so a double overflow saturates to maxpos;
// - a nonzero true result never becomes 0, so a double underflow saturates to minpos.
// The bposit conversion already saturates anything inside double's range; these helpers
// cover the results that leave it.  A pole or a domain error (log(0), sqrt(-1)) is a
// NaN or an infinity in double and stays NaR.
#include <cmath>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal { namespace bposit_shim {

// the true result is finite
template<typename BPosit>
BPosit finite(double r) {
	if (std::isinf(r)) return BPosit(r > 0.0 ? SpecificValue::maxpos : SpecificValue::maxneg);
	return BPosit(r);
}

// the true result is finite and nonzero; `negative` is its sign, needed when r is 0
template<typename BPosit>
BPosit nonzero(double r, bool negative) {
	if (r == 0.0) return BPosit(negative ? SpecificValue::minneg : SpecificValue::minpos);
	return finite<BPosit>(r);
}

template<typename BPosit>
BPosit nonzero(double r) { return nonzero<BPosit>(r, std::signbit(r)); }

}}} // namespace sw::universal::bposit_shim
