#pragma once
// attributes.hpp: definition of attribute functions for integer types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <universal/number/integer/exceptions.hpp>

namespace sw { namespace universal {

// ipow(a, b): the integer power a^b, exact, by squaring in integer's own arithmetic. A result
// past the range wraps or throws exactly as integer's operator*= does. a^0 == 1 for every a,
// 0^0 included, as std::pow has it. A negative b gives 1 / a^|b| truncated, as integer division
// truncates: 1 for a == 1, +-1 for a == -1 by the parity of b, 0 for |a| > 1, and integer's
// division by zero for a == 0.

// a^-n for n > 0, truncated; odd is the parity of n
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
integer<nbits, BlockType, NumberType> ipow_reciprocal(const integer<nbits, BlockType, NumberType>& a, bool odd) {
	using Integer = integer<nbits, BlockType, NumberType>;
	if (a.iszero()) return Integer(1) / a;   // reports the division by zero
	if (a.isone()) return a;
	if constexpr (NumberType == IntegerNumberType::IntegerNumber) {
		if (a == Integer(-1)) return odd ? a : Integer(1);
	}
	return Integer(0);
}

template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
integer<nbits, BlockType, NumberType> ipow(const integer<nbits, BlockType, NumberType>& a, const integer<nbits, BlockType, NumberType>& b) {
	using Integer = integer<nbits, BlockType, NumberType>;
	if (b.isneg()) return ipow_reciprocal(a, b.isodd());
	Integer result(1), base(a), exp(b);
	while (!exp.iszero()) {
		if (exp.isodd()) result *= base;
		exp.logicShiftRight(1);
		if (exp.iszero()) break;   // no square past the last bit: it could overflow for nothing
		base *= base;
	}
	return result;
}

// the same with a native exponent, which need not fit in the integer
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
integer<nbits, BlockType, NumberType> ipow(const integer<nbits, BlockType, NumberType>& a, long long b) {
	using Integer = integer<nbits, BlockType, NumberType>;
	if (b < 0) return ipow_reciprocal(a, (b & 1) != 0);
	Integer result(1), base(a);
	unsigned long long exp = static_cast<unsigned long long>(b);
	while (exp != 0) {
		if (exp & 1u) result *= base;
		exp >>= 1;
		if (exp == 0) break;
		base *= base;
	}
	return result;
}

}} // namespace sw::universal
