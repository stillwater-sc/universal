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
template<unsigned nbits, unsigned es, typename bt>
std::pair< posit<nbits, es, bt>, posit<nbits, es, bt> > twoSum(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b) {
#if GEOMETRIC_ROUNDING_CASES
	posit<nbits, es, bt> pminpos(SpecificValue::minpos), pmaxpos(SpecificValue::maxpos);
	if ((pminpos == a && pminpos == b) || (pmaxpos == a && pmaxpos == b)) {
		return std::pair< posit<nbits, es, bt>, posit<nbits, es, bt> >(a, b);
	}
#endif
	using Scalar = posit<nbits, es, bt>;
	Scalar s = a + b;
	Scalar aApprox = s - b;
	Scalar bApprox = s - aApprox;
	Scalar aDiff = a - aApprox;
	Scalar bDiff = b - bApprox;
	Scalar r = aDiff + bDiff;
	return std::pair<Scalar, Scalar>(s, r);
}

}} // namespace sw::universal
