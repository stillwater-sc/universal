#pragma once
// minmax.hpp: min/max functions for positional integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// minimum of two values — direct comparison, no double conversion
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix>
min(positional<ndigits, radix> x, positional<ndigits, radix> y) {
	return (x < y) ? x : y;
}

// maximum of two values — direct comparison, no double conversion
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix>
max(positional<ndigits, radix> x, positional<ndigits, radix> y) {
	return (x > y) ? x : y;
}

}} // namespace sw::universal
