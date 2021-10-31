#pragma once
// fractional.hpp: fractional functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

rational fmod(rational x, rational y) {
	return rational(std::fmod(double(x), double(y)));
}

rational remainder(rational x, rational y) {
	return rational(std::remainder(double(x), double(y)));
}

rational frac(rational x) {
	return rational(double(x)-long(x));
}


}  // namespace sw::universal
