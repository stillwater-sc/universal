#pragma once
// exponent.hpp: exponent functions for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Base-e exponential function
inline bfloat16 exp(bfloat16 x) {
	return bfloat16(std::exp(double(x)));
}

// Base-2 exponential function
inline bfloat16 exp2(bfloat16 x) {
	return bfloat16(std::exp2(double(x)));
}

// Base-10 exponential function
inline bfloat16 exp10(bfloat16 x) {
	return bfloat16(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
inline bfloat16 expm1(bfloat16 x) {
	return bfloat16(std::expm1(double(x)));
}

// ---- exponent manipulation (issue #941) ----
// bfloat16 delegates to the host double routines (matching the exp/log family
// above); a bfloat16 value is exactly representable in double, so the only loss
// is the final round of the result back to bfloat16's 8-bit significand.

// Decompose x into a normalized fraction in [0.5, 1) and an integer power of two:
// x == frexp(x, &e) * 2^e. Writes the exponent to *exp.
inline bfloat16 frexp(bfloat16 x, int* exp) {
	return bfloat16(std::frexp(double(x), exp));
}

// Multiply x by 2^n.
inline bfloat16 ldexp(bfloat16 x, int n) {
	return bfloat16(std::ldexp(double(x), n));
}

// Multiply x by FLT_RADIX^n; FLT_RADIX == 2 here, so identical to ldexp.
inline bfloat16 scalbn(bfloat16 x, int n) {
	return bfloat16(std::scalbn(double(x), n));
}

// Unbiased radix-2 exponent of x, as a floating-point value.
inline bfloat16 logb(bfloat16 x) {
	return bfloat16(std::logb(double(x)));
}

// Unbiased radix-2 exponent of x, as an int.
inline int ilogb(bfloat16 x) {
	return std::ilogb(double(x));
}

}} // namespace sw::universal
