#pragma once
// math_functions.hpp: definition of arbitrary real mathematical functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>

namespace sw { namespace universal {

// fma(a,b,c) = a*b + c, faithfully rounded into areal with the uncertainty bit.
//
// areal is a faithful floating-point encoding: the least-significant bit is the
// uncertainty bit (ubit), which marks whether a value is exact (ubit = 0) or is a
// representative of the open interval to the adjacent encoding (ubit = 1). fma forms
// the fused product-sum in a wide double intermediate and converts the result once
// into areal via the value constructor, which performs the faithful rounding:
//   - if the fused product-sum is exactly representable, the areal is exact (ubit = 0);
//   - otherwise the areal stores the truncated representative and sets ubit = 1,
//     denoting the open interval (v, v + ulp) for v > 0 (or (v - ulp, v) for v < 0)
//     that contains the true value of a*b + c.
// So the fusion is a single faithful rounding of a*b + c into areal -- there is no
// intermediate rounding to an exact areal product before adding c. Inputs are taken
// by their represented value double(x), and the IEEE special cases (inf*0 -> NaN,
// NaN / inf propagation) come from std::fma.
//
// NOTE on width: the double intermediate is rounded to nearest, and a*b + c can need more bits
// than double has: fma(minpos, minpos, 2^15) in areal<16,5> is 2^15 + 2^-48, which double rounds
// to 2^15. Converting that exact double would drop the 2^-48 without a trace, and for a true
// value just below a representable one, 2^10 - 2^-48, it would also land on the wrong side of the
// truncation. So when the product a*b is exact in double (2*(fbits+1) <= 53 and no underflow,
// which covers the practical configurations), the rounding error of the sum is recovered exactly
// with TwoSum and the areal is corrected: an error away from zero sets the ubit, an error toward
// zero steps to the inexact encoding just below the double in magnitude. Wider significands keep
// the double's rounding.
//
// Sub-issue of #1189 (universal fma). Relates to #1194.
//
// An inexact input carries over: double(x) of an areal with its ubit set is the lower end of its
// interval, so the fused result of those values is marked inexact as well. fma ignored the input
// ubits: fma(areal<16,5>(0.1f), 1, 0) was exact while 0.1 * 1 + 0 was not (#1970). Like + - * /,
// this flags the result inexact; it does not promise that the result's interval encloses the
// exact interval result (see areal_impl.hpp).
//
// The rule, as for operator*: the flag records that the result dropped bits. A factor that is an
// exact zero (not the inexact (0, minpos) encoding) makes the product exactly 0 whatever the
// other factor is, so that factor's ubit drops nothing and does not carry: fma(0.1, 0, 0) is an
// exact 0, as 0.1 * 0 + 0 is. The addend's ubit always carries (#2010).
template<unsigned nbits, unsigned es, typename bt>
areal<nbits, es, bt> fma(const areal<nbits, es, bt>& a, const areal<nbits, es, bt>& b, const areal<nbits, es, bt>& c) {
	const double da = double(a), db = double(b), dc = double(c);
	const double s = std::fma(da, db, dc);
	areal<nbits, es, bt> r(s);
	if (r.isnan() || r.isinf()) return r;   // a finite result implies finite operands
	// The correction needs exact double operands and an exact double product: 2*(fbits+1) <= 53.
	// A wider significand reaches here already rounded by double(a), and correcting the rounded
	// operands would mark an exact result inexact (areal<80,11>: fma(1 + 2^-60, 1, -2^-60) is 1).
	constexpr bool narrow = 2u * (areal<nbits, es, bt>::fbits + 1u) <= 53u;
	if (narrow && !r.at(0) && s != 0.0) {
		// r holds s exactly; recover the rounding error of s when the product is exact in double
		const double p = da * db;
		// at or above 2^-969 = 2^53 * DBL_MIN the product's rounding error is a double, so a zero
		// TwoProduct error means the product is exact
		const bool exactProduct = (std::fabs(p) >= 0x1p-969) && (std::fma(da, db, -p) == 0.0);
		if (exactProduct && std::isfinite(p)) {
			const double bv  = s - p;                              // TwoSum(p, dc): s + err == p + dc
			const double err = (p - (s - bv)) + (dc - bv);
			if (err != 0.0) {
				if ((err > 0.0) == (s > 0.0)) {
					r.set(0);   // the true value is past s, away from zero: (s, s + ulp)
				}
				else {
					// the true value is just inside s: the encoding one below s in magnitude, which
					// with its ubit set names (s - ulp, s). s is nonzero and on the grid, so a bit at
					// or above the fraction's lsb is set; borrow from the lowest one
					unsigned i = 1;
					while (!r.at(i)) ++i;
					r.reset(i);
					for (unsigned j = 0; j < i; ++j) r.set(j);
				}
			}
		}
	}
	const bool exactZeroProduct = a.iszero() || b.iszero();
	const bool inexactInput = (!exactZeroProduct && (a.at(0) || b.at(0))) || c.at(0);
	if (inexactInput) r.set(0);
	return r;
}

}} // namespace sw::universal
