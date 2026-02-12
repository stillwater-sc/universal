#pragma once
// math_minmax.hpp: min/max functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> min(posit<nbits,es,bt> x, posit<nbits, es, bt> y) {
	return (x < y) ? x : y;
}

template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> max(posit<nbits,es,bt> x, posit<nbits, es, bt> y) {
	return (x < y) ? y : x;
}

}} // namespace sw::universal
