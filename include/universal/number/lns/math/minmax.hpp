#pragma once
// minmax.hpp: min/max functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt>
min(lns<nbits, rbits, behavior, bt> x, lns<nbits, rbits, behavior, bt> y) {
	return lns<nbits, rbits, behavior, bt>(std::min(double(x), double(y)));
}

template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt>
max(lns<nbits, rbits, behavior, bt> x, lns<nbits, rbits, behavior, bt> y) {
	return lns<nbits, rbits, behavior, bt>(std::max(double(x), double(y)));
}

}} // namespace sw::universal
