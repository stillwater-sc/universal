#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for the logarithmic number system (lns)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// In a logarithmic number system the multiply is (near-)exact -- it is an addition
// of exponents -- while the ADD is the lossy step (a Gaussian-log table lookup that
// rounds). fma forms the fused product-sum a*b + c in a wide double intermediate and
// rounds the result once into lns via the value constructor, so the add-domain
// rounding happens exactly once. An lns significand is far narrower than double's 53
// bits for the practical configurations, so double(x) is the exact represented value,
// the product is exact in double, and the double -> lns rounding is the single
// rounding that determines the result. NaN propagates via the value constructor
// (lns has no infinity: isinf() is always false).
//
// LIMITATION: the double bridge assumes the operands and the exact a*b + c lie within
// binary64's range and precision. A logarithmic encoding can have a wider dynamic
// range than binary64, so operands (or a product-sum) whose magnitude exceeds ~1.8e308
// overflow to infinity in the intermediate before the fma completes. A range-safe
// log-domain fused path (working in the log/quire domain, cf. lns/fdp.hpp) is a
// possible future enhancement; this overload targets binary64-representable operands.
//
// Sub-issue of #1189 (universal fma). Relates to #1196.

#include <cmath>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> fma(const lns<nbits, rbits, bt, xtra...>& a, const lns<nbits, rbits, bt, xtra...>& b,
                                   const lns<nbits, rbits, bt, xtra...>& c) {
	return lns<nbits, rbits, bt, xtra...>(std::fma(double(a), double(b), double(c)));
}

}} // namespace sw::universal
