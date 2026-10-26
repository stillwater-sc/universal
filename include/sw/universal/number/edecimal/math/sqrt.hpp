#pragma once
// sqrt.hpp: sqrt functions for adaptive precision decimal integers
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cerrno>
#include <cstddef>
#include <universal/native/ieee754_core.hpp>
#include <universal/number/edecimal/core.hpp>            // the edecimal type itself; this header was relying on a prior include
#include <universal/number/edecimal/numeric_limits.hpp>

namespace sw { namespace universal {

	// sqrt for edecimal: the exact floor of the square root (#1725).
	//
	// edecimal is an adaptive-precision decimal integer, and its sqrt is the integer root,
	// floor(sqrt(n)), as an integer square root is everywhere else (and as this function gave
	// for small n).  It used to be edecimal(std::sqrt(double(n))), which is exact only while n
	// fits a double's 53 bits: past ~16 digits the root was wrong, and past ~308 it was inf.
	// The native branch of the EDECIMAL_NATIVE_SQRT switch referenced names that do not exist
	// and never compiled; the switch is gone.
	//
	// Newton's integer iteration x' = (x + n / x) / 2, from any x >= floor(sqrt(n)), decreases
	// strictly until it reaches floor(sqrt(n)) and then stops decreasing (with integer division
	// throughout).  The start 10^ceil(d/2), for n of d digits, is above the root.  Each step
	// roughly doubles the correct digits, so the count of steps grows with log(d).
	inline edecimal sqrt(const edecimal& n) {
		if (n.isneg()) {
#if EDECIMAL_THROW_ARITHMETIC_EXCEPTION
			throw edecimal_negative_sqrt_arg();
#else
			errno = EDOM;                             // as std::sqrt reports a domain error, with no message
			return edecimal(0);
#endif
		}
		if (n.iszero()) return n;
		const edecimal two(2);
		edecimal x(1);
		for (std::size_t i = 0; i < (n.size() + 1) / 2; ++i) x *= edecimal(10);   // 10^ceil(d/2) > sqrt(n)
		for (;;) {
			edecimal y = x;
			y += n / x;
			y /= two;
			if (!(y < x)) return x;
			x = y;
		}
	}

	// reciprocal sqrt
//	inline edecimal rsqrt(const edecimal& f) {
//		edecimal rsqrt = sqrt(f);
//		return rsqrt.reciprocate();
//	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
