#pragma once
// pow.hpp: integer power function for positional integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// ipow(a, b): the integer power a^b, exact, by squaring in positional's own arithmetic. A result
// past the range behaves as positional's operator*= does. a^0 == 1 for every a, 0^0 included, as
// std::pow has it. A negative b gives 1 / a^|b| truncated, as positional division truncates: 1 for
// a == 1, +-1 for a == -1 by the parity of b, 0 for |a| > 1, and positional's division by zero for
// a == 0.
template<unsigned ndigits, unsigned radix>
positional<ndigits, radix> ipow(positional<ndigits, radix> base, unsigned exp) {
	positional<ndigits, radix> result(1);
	while (exp > 0) {
		if (exp & 1) {
			result *= base;
		}
		exp >>= 1;
		if (exp == 0) break;   // no square past the last bit: it could overflow for nothing
		base *= base;
	}
	return result;
}

template<unsigned ndigits, unsigned radix>
positional<ndigits, radix> ipow(const positional<ndigits, radix>& a, const positional<ndigits, radix>& b) {
	using Positional = positional<ndigits, radix>;
	const Positional one(1), two(2);
	if (b.isneg()) {
		if (a.iszero()) return one / a;   // reports the division by zero
		if (a == one) return a;
		if (a == -one) return (b % two).iszero() ? one : a;
		return Positional(0);
	}
	Positional result(1), base(a), exp(b);
	while (!exp.iszero()) {
		if (!(exp % two).iszero()) result *= base;
		exp /= two;
		if (exp.iszero()) break;
		base *= base;
	}
	return result;
}

}} // namespace sw::universal
