#pragma once
#include <cerrno>
// sqrt.hpp: sqrt functions for logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

namespace sw { namespace universal {

	/*
	- Consider the function argument, x, in floating-point form, with a base
	(or radix) B, exponent e, and a fraction, f , such that 1/B <= f < 1.
	Then we have x = f Be. The number of bits in the exponent and
	fraction, and the value of the base, depends on the particular floating
	point arithmetic system chosen.

	- Use properties of the elementary function to range reduce the argument
	x to a small fixed interval.

	- Use a small polynomial approximation to produce an initial estimate,
	y0, of the function on the small interval. Such an estimate may
	be good to perhaps 5 to 10 bits.

	- Apply Newton iteration to refine the result. This takes the form yk =
	yk?1/2 + (f /2)/yk?1. In base 2, the divisions by two can be done by
	exponent adjustments in floating-point computation, or by bit shifting
	in fixed-point computation.

	Convergence of the Newton method is quadratic, so the number of
	correct bits doubles with each iteration. Thus, a starting point correct
	to 7 bits will produce iterates accurate to 14, 28, 56, ... bits. Since the
	number of iterations is very small, and known in advance, the loop is
	written as straight-line code.

	- Having computed the function value for the range-reduced argument,
	make whatever adjustments are necessary to produce the function value
	for the original argument; this step may involve a sign adjustment,
	and possibly a single multiplication and/or addition.
	*/


	// sqrt for lns, correctly rounded (#1725).
	//
	// An lns value is 2^(E 2^-rbits) for a two's complement fixed-point exponent E, and lns
	// rounds in the exponent domain: converting a real v stores log2(v) rounded to the nearest
	// multiple of 2^-rbits, ties to even.  The root is 2^(E/2 2^-rbits), so the correctly
	// rounded root under that rule is E/2 rounded to an integer, ties to even, computed here
	// exactly by an arithmetic shift: an odd E is an exact tie.  Halving moves E towards zero,
	// so the result can neither overflow nor reach the zero encoding (the most negative E).
	//
	// This used to be lns(std::sqrt(double(a))) in both branches of an LNS_NATIVE_SQRT switch,
	// one of them marked "TBD": the root of a value through double, its log2 through double, and
	// a second rounding into the exponent.  That capped the precision at a double's and decided
	// every tie by the noise in std::log2.
	template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
	inline lns<nbits, rbits, bt, xtra...> sqrt(const lns<nbits, rbits, bt, xtra...>& a) {
		using Lns = lns<nbits, rbits, bt, xtra...>;
		if (a.isnan()) return a;                      // the NaN encoding carries the sign bit: test it first
		if (a.iszero()) return a;
		if (a.isneg()) {
#if LNS_THROW_ARITHMETIC_EXCEPTION
			throw lns_negative_sqrt_arg();
#else
			errno = EDOM;                             // as std::sqrt: EDOM and a NaN, with no message
			Lns nan;
			nan.setnan();
			return nan;
#endif
		}
		constexpr unsigned ebits = nbits - 1u;        // the exponent field, bits [0, ebits)
		Lns r(a);
		const bool odd = a.at(0);
		for (unsigned i = 0; i + 1u < ebits; ++i) r.setbit(i, a.at(i + 1u));   // arithmetic shift right
		r.setbit(ebits - 1u, a.at(ebits - 1u));      // the exponent's sign bit stays
		if (odd && r.at(0)) {                         // a tie onto an odd exponent: round up to even
			for (unsigned i = 0; i < ebits; ++i) {    // two's complement increment
				const bool bit = r.at(i);
				r.setbit(i, !bit);
				if (!bit) break;
			}
		}
		r.setsign(false);
		return r;
	}

	// reciprocal sqrt
	template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
	inline lns<nbits, rbits, bt, xtra...> rsqrt(const lns<nbits, rbits, bt, xtra...>& a) {
		lns<nbits, rbits, bt, xtra...> v = sqrt(a);
		return v.reciprocate();
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
