#pragma once
// isrepresentable.hpp: test to see if a ratio of Integer type can be represented by a binary real
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>

namespace sw { namespace universal {

/*
A number can be exactly represented in base 10 if the prime factorization of the denominator consists of only 2's and 5's.

A rational number X can be exactly represented in base N if the prime factorization of the denominator of X contains only primes found in the factorization of N.
*/

// isRepresentable tests if the ratio a/b can be represented exactly by a binary Real
template<typename IntegerType>
bool isRepresentable(IntegerType a, IntegerType b) {
	if (b == 0) return false;
	while (b % 2 == 0) { b /= 2; }
	while (b % 5 == 0) { b /= 5; }
	return a % b == 0;
}

template<typename IntegerType>
void reportRepresentability(IntegerType a, IntegerType b) {
	std::cout << a << "/" << b << (isRepresentable(a, b) ? " is    " : " is not") << " representable " << ((long double)a / (long double)(b)) << std::endl;
}

// given a string representation of a decimal scientific number, report if it can be represented in binary floating-point
bool isRepresentableInBinary(const std::string& scientificDecimalNumber) {
	if (size(scientificDecimalNumber) > 0) {
	}
	return true;
}

}} // namespace sw::universal
