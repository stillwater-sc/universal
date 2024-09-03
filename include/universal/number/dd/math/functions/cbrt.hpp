#pragma once
// cbrt.hpp: cbrt function for double-double floating-point
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
	inline dd cbrt(const dd& a) {
		using std::pow;
		if (!a.isfinite() || a.iszero())
			return a;						//	NaN returns NaN; +/-Inf returns +/-Inf, +/-0.0 returns +/-0.0

		bool signA = signbit(a);
		int e;								//	0.5 <= r < 1.0
		dd r = frexp(abs(a), &e);
		while (e % 3 != 0) {
			++e;
			r = ldexp(r, -1);
		}

		// at this point, 0.125 <= r < 1.0
		dd x = pow(r.high(), -dd_third.high());

		//	refine estimate using Newton's iteration
		x += x * (1.0 - r * sqr(x) * x) * dd_third;
		x += x * (1.0 - r * sqr(x) * x) * dd_third;
		x = reciprocal(x);

		if (signA)
			x = -x;

		return ldexp(x, e / 3);
	}

}} // namespace sw::universal
