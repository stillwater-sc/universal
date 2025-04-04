#pragma once
// twosum.hpp: definition of the twoSum function
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

/*
TwoSum denotes an algorithm introduced by Knuth in "The Art of Computer Programming", vol 2, Seminumerical Algorithms.

Given two floating-point values a and b, 
generate a rounded sum s and a remainder r, such that
s = RoundToNearest(a + b), and
a + b = s + r

floating-point arithmetic property : Sterbenz lemma
	- float(x - y) = x - y when x / 2 <= y <= 2x : difference is represented exactly when two numbers are less than 2x of each other
	- float(2x) = 2x barring overflow
	- float(x / 2) = x / 2 barring underflow
*/

/// 
// 

//#define SW_VOLATILE volatile
#define SW_VOLATILE 

/// <summary>
/// For any faithful number system, twoSum generates the relationship a + b = s + r, 
/// where s is the sum, and r is the remainder.
/// All arguments must be distinct variables, 
/// so for example you can't do this: 
///      twoSum(s, bprime, s, rprime) as s is being used as both input and output.
/// </summary>
/// <typeparam name="Scalar"></typeparam>
/// <param name="a">input operand</param>
/// <param name="b">input operand</param>
/// <param name="s">output resulting sum</param>
/// <param name="r">output resulting error</param>	
template<typename Scalar>
void twoSum(const Scalar& a, const Scalar& b, Scalar& s, Scalar& r) {
	// the volatile keyword is required to avoid the operation being  
	// optimized away by a compiler using associativity rules
	// when does this requirement exist and when does it not? 
	// MSVC 2019 seems fine without it: ETLO 6/9/2023
	// s = a + b
	// r = aerr + berr = (a - aDelta) + (b  - bDelta) 
	//   = (a - s + bDelta) + (b - s + a) 
	//   = (a - s + s - a) + (b - s + a)
	s = a + b;
	Scalar bDelta = s - a;
	Scalar aDelta = s - bDelta;
	SW_VOLATILE Scalar aerr = a - aDelta;
	SW_VOLATILE Scalar berr = b - bDelta;
	r = aerr + berr;
}

/// <summary>
/// cascadingSum generates a cumulative twoSum on a vector
/// </summary>
/// <typeparam name="Vector"></typeparam>
/// <typeparam name="Scalar"></typeparam>
/// <param name="v">input vector</param>
/// <param name="s">output resulting sum</param>
/// <param name="r">output resulting remainder</param>
template<typename Vector, typename Scalar>
void cascadingSum(const Vector& v, Scalar& s, Scalar& r) {
	Scalar a, p, q;
	size_t N = v.size();
	p = v[0];
	r = 0;
	for (size_t i = 1; i < N; ++i) {
		a = p;
		twoSum(a, v[i], p, q);
		r += q;
	}
	s = p;
}

}} // namespace sw::universal
