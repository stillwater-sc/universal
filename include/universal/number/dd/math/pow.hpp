#pragma once
// pow.hpp: pow functions for doubledouble (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

dd pow(dd x, dd y) {
	using std::pow;
	return dd(std::pow(double(x), double(y)));
}
		
dd pow(dd x, int y) {
	using std::pow;
	return dd(std::pow(double(x), double(y)));
}
		
dd pow(dd x, double y) {
	using std::pow;
	return dd(std::pow(double(x), y));
}

}} // namespace sw::universal
