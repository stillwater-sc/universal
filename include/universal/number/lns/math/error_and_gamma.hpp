#pragma once
// error_gamma.hpp: error and gamma functions for lns
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> erf(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> erfc(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::erfc(double(x)));
}

}} // namespace sw::universal
