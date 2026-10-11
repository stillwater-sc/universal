#pragma once
// shim.hpp: rounding a double result into a b-posit, saturating as the posit standard requires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 evaluated every function in double and rounded the result once.  The elementary
// functions are now correctly rounded (correctly_rounded.hpp), and this evaluation remains
// for erf, erfc, tgamma and lgamma, and for configurations whose range is beyond the
// quad-double engine.  It is NOT correctly rounded: libm's double result is within about an
// ulp of the true value, so the result is correct except when the true value lies that close
// to a rounding boundary of the bposit, and then it can be one encoding off.  The argument
// converts to double exactly when the significand fits (fbits + 1 <= 53) and the range does.
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
