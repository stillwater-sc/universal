#pragma once
// pow.hpp: pow functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> pow(lns<nbits, rbits, behavior, bt> x, lns<nbits, rbits, behavior, bt> y) {
	return lns<nbits, rbits, behavior, bt>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> pow(lns<nbits, rbits, behavior, bt> x, int y) {
	return lns<nbits, rbits, behavior, bt>(std::pow(double(x), double(y)));
}
		
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> pow(lns<nbits, rbits, behavior, bt> x, double y) {
	return lns<nbits, rbits, behavior, bt>(std::pow(double(x), y));
}

}} // namespace sw::universal
