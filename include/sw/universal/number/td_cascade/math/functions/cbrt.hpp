#pragma once
// cbrt.hpp: cube root for triple-double cascade (td_cascade) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	/// <summary>
	/// cbrt is cube root function, that is, x^1/3
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>cube root of a</returns>
	inline td_cascade cbrt(const td_cascade& a) {
		using std::pow;
		if (!a.isfinite() || a.iszero())
			return a;						//	NaN returns NaN; +/-Inf returns +/-Inf, +/-0.0 returns +/-0.0

		bool signA = signbit(a);
		int e;								//	0.5 <= r < 1.0
	    td_cascade r = frexp(abs(a), &e);
		while (e % 3 != 0) {
			++e;
			r = ldexp(r, -1);
		}

		// at this point, 0.125 <= r < 1.0
	    td_cascade x = pow(r[0], -tdc_third[0]);

		//	refine estimate using Newton's iteration
		x += x * (1.0 - r * sqr(x) * x) * tdc_third;
		x += x * (1.0 - r * sqr(x) * x) * tdc_third;
		x = reciprocal(x);

		if (signA)
			x = -x;

		return ldexp(x, e / 3);
	}

}} // namespace sw::universal
