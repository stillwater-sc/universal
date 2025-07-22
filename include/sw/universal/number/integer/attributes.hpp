#pragma once
// attributes.hpp: definition of attribute functions for integer types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <universal/number/integer/exceptions.hpp>

namespace sw { namespace universal {

// exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography

// calculate the integer power a ^ b using exponentiation by squaring
template<unsigned nbits, typename BlockType>
integer<nbits, BlockType> ipow(const integer<nbits, BlockType>& a, const integer<nbits, BlockType>& b) {
	integer<nbits, BlockType> result(1), base(a), exp(b);
	for (;;) {
		if (exp.isodd()) result *= base;
		exp.logicShiftRight(1);
		if (exp == 0) break;
		base *= base;
	}
	return result;
}

}} // namespace sw::universal
