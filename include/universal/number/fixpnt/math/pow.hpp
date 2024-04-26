#pragma once
// pow.hpp: pow functions for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> pow(fixpnt<nbits, rbits, arithmetic, bt> x, fixpnt<nbits, rbits, arithmetic, bt> y) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::pow(double(x), double(y)));
}
		
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> pow(fixpnt<nbits, rbits, arithmetic, bt> x, int y) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::pow(double(x), double(y)));
}
		
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> pow(fixpnt<nbits, rbits, arithmetic, bt> x, double y) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::pow(double(x), y));
}

}} // namespace sw::universal
