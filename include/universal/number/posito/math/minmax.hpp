#pragma once
// math_minmax.hpp: min/max functions for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posito standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

template<unsigned nbits, unsigned es>
posito<nbits,es> min(posito<nbits,es> x, posito<nbits, es> y) {
	return (x < y) ? x : y;
}

template<unsigned nbits, unsigned es>
posito<nbits,es> max(posito<nbits,es> x, posito<nbits, es> y) {
	return (x < y) ? y : x;
}

}} // namespace sw::universal
