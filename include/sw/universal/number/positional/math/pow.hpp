#pragma once
// pow.hpp: integer power function for positional integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// integer exponentiation by squaring (native, no double conversion)
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix> ipow(positional<ndigits, radix> base, unsigned exp) {
	positional<ndigits, radix> result(1);
	while (exp > 0) {
		if (exp & 1) {
			result *= base;
		}
		base *= base;
		exp >>= 1;
	}
	return result;
}

}} // namespace sw::universal
