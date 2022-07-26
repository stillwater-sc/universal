#pragma once
// error_gamma.hpp: error and gamma functions for lns
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> erf(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<size_t nbits, size_t rbits, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
lns<nbits, rbits, bt> erfc(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::erfc(double(x)));
}

}} // namespace sw::universal
