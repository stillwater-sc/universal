#pragma once
// twosum.hpp: definition of the twoSum function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <tuple>

namespace sw::function {

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
template<typename Scalar>
void twoSum(const Scalar& a, const Scalar& b, Scalar& s, Scalar& r) {
	s = a + b;
	Scalar bdiff = s - a;
	Scalar adiff = s - bdiff;
	std::cout << "adiff " << adiff << '\n';
	std::cout << "bdiff " << bdiff << '\n';
	Scalar aerr = a - adiff;
	Scalar berr = b - bdiff;
	std::cout << "aerr " << aerr << '\n';
	std::cout << "berr " << berr << '\n';
	r = aerr + berr;
}

template<typename Scalar>
std::pair<Scalar, Scalar> twoSum(const Scalar& a, const Scalar& b) {
	Scalar s = a + b;
	Scalar aApproximate = s - b;
	Scalar bApproximate = s - aApproximate;
	Scalar aDiff = a - aApproximate;
	Scalar bDiff = b - bApproximate;
	Scalar r = aDiff + bDiff;
	return std::make_pair(s, r);
}

template<typename Scalar>
void cascadingSum(const std::vector<Scalar>& v, Scalar& s, Scalar& r) {
	Scalar p, q;
	size_t N = v.size();
	p = v[0];
	r = 0;
	for (size_t i = 1; i < N; ++i) {
		twoSum(p, v[i], s, q);
		p = s;
		r += q;
		std::cout << s << " + " << r << '\n';
	}
}

}  // namespace sw::function