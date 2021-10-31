#pragma once
// minmax.hpp: min/max functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

rational min(rational x, rational y) {
	return rational(std::min(double(x), double(y)));
}

rational max(rational x, rational y) {
	return rational(std::max(double(x), double(y)));
}

}  // namespace sw::universal
