#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for integer
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Integer arithmetic is exact -- there is no rounding, so the fused-vs-two-rounding
// distinction that motivates fma for floating-point types is a no-op here. And
// because fixed-size integer arithmetic is modular, wrapping the product before
// adding c yields the same value as wrapping once at the end: (a*b + c) mod 2^nbits.
// The overload exists so generic ADL code (dot products, polynomial evaluation,
// ...) that calls fma compiles and runs over integer with the expected semantics.
//
// Sub-issue of #1189 (universal fma). Relates to #1192.

namespace sw { namespace universal {

// fma(a,b,c) = a*b + c, exact modulo the integer's width (same overflow behavior as a*b and a+b).
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
integer<nbits, BlockType, NumberType>
fma(const integer<nbits, BlockType, NumberType>& a,
    const integer<nbits, BlockType, NumberType>& b,
    const integer<nbits, BlockType, NumberType>& c) {
	return a * b + c;
}

}} // namespace sw::universal
