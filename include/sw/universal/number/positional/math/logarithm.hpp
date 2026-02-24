#pragma once
// logarithm.hpp: logarithm functions for positional integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

// Natural logarithm of x
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix> log(positional<ndigits, radix> x) {
	return positional<ndigits, radix>(::std::log(double(x)));
}

// Binary logarithm of x
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix> log2(positional<ndigits, radix> x) {
	return positional<ndigits, radix>(::std::log2(double(x)));
}

// Decimal logarithm of x
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix> log10(positional<ndigits, radix> x) {
	return positional<ndigits, radix>(::std::log10(double(x)));
}

}} // namespace sw::universal
