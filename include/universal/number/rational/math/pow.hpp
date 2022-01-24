#pragma once
// pow.hpp: pow functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

rational pow(rational x, rational y) {
	return rational(std::pow(double(x), double(y)));
}
		
rational pow(rational x, int y) {
	return rational(std::pow(double(x), double(y)));
}
		
rational pow(rational x, double y) {
	return rational(std::pow(double(x), y));
}

}} // namespace sw::universal
