#pragma once
// exponent.hpp: exponent functions for lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>         // std::exp
namespace sw { namespace universal {

// exp and exp2 are correctly rounded at every width (#2128): lns_rounding::exponential
// (lns_correct_rounding.hpp) decides the nearest exponent by Ziv's strategy, a double fast path where
// it is clear of a tie and efloat elsewhere. An underflow follows the rounding rule, zero below half
// of minpos, as conversion and multiplication do (USER decision on #2128). They used to be
// std::exp(double(x)) converted back: wrong by up to 972 units at lns<64,59>, and minpos where double
// underflowed but zero just above it, so not monotone. exp10 and expm1 below are still the old shims,
// not correctly rounded.

// Base-e exponential function
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> exp(lns<nbits, rbits, bt, xtra...> x) {
	return lns_rounding::exponential('e', x);
}

// Base-2 exponential function
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> exp2(lns<nbits, rbits, bt, xtra...> x) {
	return lns_rounding::exponential('2', x);
}

// Base-10 exponential function
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> exp10(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> expm1(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::expm1(double(x)));
}


}} // namespace sw::universal
