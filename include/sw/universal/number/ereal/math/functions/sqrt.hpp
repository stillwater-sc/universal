#pragma once
// sqrt.hpp: sqrt function for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <limits>

namespace sw { namespace universal {

	// sqrt: square root
	//
	// Karp's method: Newton on the reciprocal square root, y' = y + y (1 - m y^2) / 2, which
	// needs no division, then one step on the root itself, s' = s + y (m - s^2) / 2 with s = m y,
	// which doubles the last y's precision (A. H. Karp and P. Markstein, "High-precision
	// division and square root", ACM TOMS 23(4), 1997).  The argument is reduced exactly to
	// m in [0.5, 2) and an even power of two.  Each step doubles the bits from the limb seed
	// until half of what the result can carry, the limb budget, is reached.
	//
	// It replaced 3 + log2(maxlimbs + 1) Newton steps x' = (x + a / x) / 2, each a full
	// expansion division: 1.9 ms at ereal<8>, against 0.48 ms now, for the same digits (#2006).
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> sqrt(const ereal<maxlimbs, FpType>& a) {
		using Real = ereal<maxlimbs, FpType>;
		// C99 F.10.4.5: sqrt(+-0) = +-0, sqrt(+inf) = +inf, and NaN for NaN and for any
		// argument below zero, -inf included.  sqrt(-1) used to return -1, and sqrt(+inf) NaN
		// from inf / inf in the Newton step (#2006).
		if (a.iszero() || a.isnan()) return a;
		if (a.isneg()) return ereal_detail::quiet_nan<maxlimbs, FpType>();
		if (a.isinf()) return a;

		int e = 0;
		Real m = frexp(a, &e);                 // a = m 2^e, m in [0.5, 1)
		if (e & 1) { m = ldexp(m, 1); --e; }   // e even, m in [0.5, 2)

		constexpr int digits = std::numeric_limits<FpType>::digits;
		constexpr int target = static_cast<int>(Real::limb_budget) * digits + 8;
		const Real half(0.5), one(1.0);
		Real y(FpType(1) / std::sqrt(m.limbs()[0]));
		for (int bits = digits - 2; 2 * bits < target; bits *= 2) {
			y = y + y * (one - m * y * y) * half;
		}
		Real s = m * y;
		s = s + y * (m - s * s) * half;

		// An exact root is returned exactly: sqrt(25/16) is 1.25, not 1.25 less 2^-400, so
		// hypot(3, 4) is 5.  The iteration leaves a trailing error that the limb budget cannot
		// resolve, so test the leading limbs of s, whose square is exact, against m.  The old
		// Newton loop was not exact on squares either.  Short roots are the ones that occur, so
		// three limbs bound the cost.
		Real t(0.0);
		const auto& sl = s.limbs();
		for (std::size_t j = 0; j < sl.size() && j < 3; ++j) {
			t += Real(sl[j]);
			if (t * t == m) { s = t; break; }
		}
		return ldexp(s, e / 2);
	}

}} // namespace sw::universal
