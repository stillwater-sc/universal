#pragma once
// fma.hpp: fused multiply-add fma(a,b,c) = a*b + c for bfloat16 (Google Brain float)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Single-rounding fused multiply-add, correctly rounded to bfloat16.
//
// The product of two bfloat16 values has at most 16 significant bits and a range of 2^+-266,
// so it is exact in a double. The sum p + c is formed with TwoSum: s = fl(p + c) and the exact
// error e, so s + e == a*b + c. When e is not zero, s is moved to its odd neighbour on e's side
// (round-to-odd to 53 bits), and bfloat16(double) narrows with round-to-odd again and rounds to
// nearest, ties to even, once. Round-to-odd keeps every rounding but the last innocuous.
//
// This used to be bfloat16(std::fma(float(a), float(b), float(c))): rounding the exact result
// to float first can land it on a bfloat16 tie, and the tie then rounds to even although the
// exact value was on one side of it. fma(-9.3306e-22, -2.90142e+25, -1.97887e-05) has the
// product 27072, the tie between 27008 and 27136, and a small negative addend: it came out
// 27136, not 27008 (#2106). The "24 >= 2*8 + 2" argument for innocuous double rounding holds
// for +, -, *, / and sqrt, not for fma, whose exact result can carry any number of bits.
//
// Special values follow IEEE: a non-finite product or sum (inf * 0, inf - inf, NaN) is
// returned as computed in double, and +0 + -0 is +0.
//
// Sub-issue of #1189 (universal fma). Relates to #1193.

#include <cmath>         // std::isfinite, std::nextafter
#include <cstdint>
#include <cstring>
#include <limits>

namespace sw { namespace universal {

inline bfloat16 fma(bfloat16 a, bfloat16 b, bfloat16 c) {
	const double p  = double(a) * double(b);          // exact
	const double cd = double(c);
	const double s  = p + cd;
	// inf or NaN: classified, not tested by s - s, which raises FE_INVALID for an infinity
	if (!std::isfinite(p) || !std::isfinite(cd) || !std::isfinite(s)) return bfloat16(s);
	// TwoSum: s + e == p + cd exactly
	const double bv = s - p;
	const double e  = (p - (s - bv)) + (cd - bv);
	double r = s;
	if (e != 0.0) {
		std::uint64_t bits = 0;
		std::memcpy(&bits, &r, sizeof(bits));
		if ((bits & 1u) == 0u) r = std::nextafter(r, (e > 0.0) ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity());
	}
	return bfloat16(r);
}

}} // namespace sw::universal
