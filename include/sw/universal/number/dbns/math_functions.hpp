#pragma once
// math_functions.hpp: definition of 2-base logarithmic number system mathematical functions
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>

namespace sw { namespace universal {

// fma(a,b,c) = a*b + c for the double-base logarithmic number system (dbns).
//
// In a (double-base) logarithmic number system the multiply is (near-)exact while the
// ADD is the lossy step. fma forms the fused product-sum in a wide double intermediate
// and rounds the result once into dbns via the value constructor, so the add-domain
// rounding happens exactly once. A dbns significand is far narrower than double's 53
// bits for the practical configurations, so double(x) is the exact represented value,
// the product is exact in double, and the double -> dbns rounding is the single
// rounding that determines the result. NaN propagates via the value constructor
// (dbns has no infinity: isinf() is always false).
//
// Sub-issue of #1189 (universal fma). Relates to #1196.
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
dbns<nbits, fbbits, bt, xtra...> fma(const dbns<nbits, fbbits, bt, xtra...>& a,
                                     const dbns<nbits, fbbits, bt, xtra...>& b,
                                     const dbns<nbits, fbbits, bt, xtra...>& c) {
	return dbns<nbits, fbbits, bt, xtra...>(std::fma(double(a), double(b), double(c)));
}

}} // namespace sw::universal
