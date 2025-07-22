#pragma once
// twoSum.hpp :  TwoSum specialization for posit number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Default is to return a and b in exponential range of posits
#ifndef GEOMETRIC_ROUNDING_CASES
#define GEOMETRIC_ROUNDING_CASES 1
#endif

/*
floating point arithmetic:
 - integers are represented exactly
 - float(x - y) = x - y when x/2 <= y <= 2x: difference is represented exactly when two numbers are less than 2x of each other
 - float(2x)    = 2x barring overflow
 - float(x/2)   = x/2 barring underflow

TwoSum denotes an algorithm introduced by Knuth in "The Art of Computer Programming", vol 2, Seminumerical Algorithms.

Given two floating point values a and b, generate a rounded sum s and a remainder r, such that
s = RoundToNearest(a + b), and
a + b = s + r
*/
template<unsigned nbits, unsigned es>
std::pair< posit<nbits, es>, posit<nbits, es> > twoSum(const posit<nbits, es>& a, const posit<nbits, es>& b) {
#if GEOMETRIC_ROUNDING_CASES
	posit<nbits, es> pminpos(SpecificValue::minpos), pmaxpos(SpecificValue::maxpos);
	if ((pminpos == a && pminpos == b) || (pmaxpos == a && pmaxpos == b)) {
		return std::pair< posit<nbits, es>, posit<nbits, es> >(a, b);
	}
#endif
	using Scalar = posit<nbits, es>;
	Scalar s = a + b;
	Scalar aApprox = s - b;
	Scalar bApprox = s - aApprox;
	Scalar aDiff = a - aApprox;
	Scalar bDiff = b - bApprox;
	Scalar r = aDiff + bDiff;
	return std::pair<Scalar, Scalar>(s, r);
}

	/*
		when rounding of s falls in the geometric rounding region, there doesn't exist an r which satisfies s+r=a+b.

		So for add_exact, if we determine that finding the best s,r is too complex for a subset of values,
		we can define the standard as returning ( max(|a|,|b|), min(|a|,|b|) ) in those scenarios
		and it will probably just be a bit less efficient summing lists (though we need to verify it doesn't
		get into silly states).

		However, because it's going into a mining algorithm, whatever we define as the rule now is what all
		hardware will end up targeting... "When you make a bug in blockchain code, people write books about it"
		so we should make a reasonable effort to find (s,r) of the smallest r where s+r=a+b.

	template<unsigned nbits, unsigned es>
	std::pair< posit<nbits, es>, posit<nbits, es> > add_exact(const posit<nbits, es>& a, const posit<nbits, es>& b) {
		using Scalar = posit<nbits, es>;

		if (abs(b) > abs(a)) { return add_exact(b, a); }
		temp = posit<nbits * 2, es>(a) + posit<nbits * 2, es>(b)

		// no bits intersect
		if (posit<nbits, es>(temp) == a) { return std::pair<Scalar, Scalar>(a, b); }

		// bits intersect, therefore we believe that temp is added exactly
		s = posit<nbits, es>::convert_truncate_bits(temp);
		temp = temp - posit<nbits * 2, es>(s);
		r = posit<nbits, es>::convert_assert_exact(temp);
		return  std::pair<Scalar, Scalar>(s, r);
	}
	*/

}} // namespace sw::universal

