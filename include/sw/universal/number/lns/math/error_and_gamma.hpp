#pragma once
// error_gamma.hpp: error and gamma functions for lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> erf(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> erfc(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::erfc(double(x)));
}

}} // namespace sw::universal
