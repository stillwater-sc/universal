#pragma once
// math_frac.hpp: fractional functions for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posito standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

template<unsigned nbits, unsigned es>
posito<nbits,es> fmod(posito<nbits,es> x, posito<nbits,es> y) {
	return posito<nbits,es>(std::fmod(double(x), double(y)));
}

template<unsigned nbits, unsigned es>
posito<nbits,es> remainder(posito<nbits,es> x, posito<nbits,es> y) {
	return posito<nbits,es>(std::remainder(double(x), double(y)));
}

template<unsigned nbits, unsigned es>
posito<nbits,es> frac(posito<nbits,es> x) {
	return posito<nbits,es>(double(x)-long(x));
}


}} // namespace sw::universal
