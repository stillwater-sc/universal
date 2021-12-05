#pragma once
// twosum.hpp: definition of the twoSum function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <tuple>

#undef TRACE_TWOSUM

namespace sw::universal {

/*
TwoSum denotes an algorithm introduced by Knuth in "The Art of Computer Programming", vol 2, Seminumerical Algorithms.

Given two floating point values a and b, generate a rounded sum s and a remainder r, such that
s = RoundToNearest(a + b), and
a + b = s + r

floating point arithmetic :
	- float(x - y) = x - y when x / 2 <= y <= 2x : difference is represented exactly when two numbers are less than 2x of each other
	- float(2x) = 2x barring overflow
	- float(x / 2) = x / 2 barring underflow
*/
#ifdef TRACE_TWOSUM
template<typename Scalar>
void twoSum(const Scalar& a, const Scalar& b, Scalar& s, Scalar& r) {
	std::cout << "twosum\n";
	std::cout << "a     " << a << '\n';
	std::cout << "b     " << b << '\n';
	s = a + b;
	std::cout << "s     " << s << '\n';
	Scalar bdiff = s - a;
	Scalar adiff = s - bdiff;
	std::cout << "adiff " << adiff << '\n';
	std::cout << "bdiff " << bdiff << '\n';
	volatile Scalar aerr = a - adiff;
	volatile Scalar berr = b - bdiff;
	std::cout << "aerr " << aerr << '\n';
	std::cout << "berr " << berr << '\n';
	r = aerr + berr;
}

template<typename Scalar>
void cascadingSum(const std::vector<Scalar>& v, Scalar& s, Scalar& r) {
	Scalar a, p, q;
	size_t N = v.size();
	p = v[0];
	r = 0;
	for (size_t i = 1; i < N; ++i) {
		a = p;
		twoSum(a, v[i], p, q);
		r += q;
		std::cout << "stage " << i << " : " << a << " + " << b << " = " << p << " + " << q << " cumulative err: " << r << '\n';
	}
	s = p;
}

#else

// twoSum generates the relationship a + b = s + r, where s is the sum, and r is the remainder, for any faithful number system
// All arguments must be distinct variables, so for example you can't do this: twoSum(s, bprime, s, rprime) as s is being used as both input and output.
template<typename Scalar>
void twoSum(const Scalar& a, const Scalar& b, Scalar& s, Scalar& r) {
	s = a + b;
	Scalar bdiff = s - a;
	Scalar adiff = s - bdiff;
	volatile Scalar aerr = a - adiff;
	volatile Scalar berr = b - bdiff;
	r = aerr + berr;
}

// cascadingSum generates a cumulative twoSum on a vector
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
#endif

}  // namespace sw::universal