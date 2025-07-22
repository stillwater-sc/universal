#pragma once
// math_frac.hpp: fractional functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

template<unsigned nbits, unsigned es>
posit<nbits,es> fmod(posit<nbits,es> x, posit<nbits,es> y) {
	return posit<nbits,es>(std::fmod(double(x), double(y)));
}

template<unsigned nbits, unsigned es>
posit<nbits,es> remainder(posit<nbits,es> x, posit<nbits,es> y) {
	return posit<nbits,es>(std::remainder(double(x), double(y)));
}

template<unsigned nbits, unsigned es>
posit<nbits,es> frac(posit<nbits,es> x) {
	return posit<nbits,es>(double(x)-long(x));
}


}} // namespace sw::universal
