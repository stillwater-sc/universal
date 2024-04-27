#pragma once
// fractional.hpp: fractional functions for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> fmod(fixpnt<nbits, rbits, arithmetic, bt> x, fixpnt<nbits, rbits, arithmetic, bt> y) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::fmod(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> remainder(fixpnt<nbits, rbits, arithmetic, bt> x, fixpnt<nbits, rbits, arithmetic, bt> y) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::remainder(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> frac(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(double(x)-long(x));
}


}} // namespace sw::universal
