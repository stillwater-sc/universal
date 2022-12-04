#pragma once
// pow.hpp: pow functions for brain floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

bfloat16 pow(bfloat16 x, bfloat16 y) {
	return bfloat16(std::pow(double(x), double(y)));
}
		
bfloat16 pow(bfloat16 x, int y) {
	return bfloat16(std::pow(double(x), double(y)));
}
		
bfloat16 pow(bfloat16 x, double y) {
	return bfloat16(std::pow(double(x), y));
}

}} // namespace sw::universal
