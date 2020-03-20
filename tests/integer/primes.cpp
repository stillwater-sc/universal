// primes.cpp: prime finding tests
//
// Binomial coefficients are useful to generate the inverse of a Hilbert matrix
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/integer/integer>
#include <universal/integer/math_functions.hpp>
#include <universal/integer/primes.hpp>

// conditional compilation
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main() 
try {
	using namespace std;
	using namespace sw::unum;

#if MANUAL_TESTING

	constexpr size_t nbits = 1024;
	using BlockType = uint32_t;
	using Integer = integer<nbits, BlockType>;
	Integer a, b, c;
	vector<Integer> v;

	cout << "\nFind all prime numbers in a range\n";
	v.clear();
	a = 2; b = 100;
	primeNumbersInRange(a, b, v);
	cout << v.size() << " prime numbers in range [" << a << ", " << b << ")" << endl;

	cout << "\nCheck primeness of a couple of values around 1k\n";
	a = 1024 + 1;
	for ( a = 1025; a < 1050; ++a) {
		cout << a << (isPrime(a) ? " is a prime number" : " is not a prime number") << endl;
		++a;
	}

	// test Fermat's method
	cout << "\nFermat's factorization\n";
	a = 1024;
	for (Integer i = a + 1; i < a + 25; i += 2) {
		cout << i << " " << fermatFactorization(i) << endl;
	}

	cout << "\nFind all prime factors of a number\n";
	// find all prime factors of a number
	a = ipow(Integer(2), Integer(5))
		* ipow(Integer(3), Integer(4))
		* ipow(Integer(5), Integer(3))
		* ipow(Integer(7), Integer(2))
		* ipow(Integer(11), Integer(1))
		* ipow(Integer(13), Integer(1))
		* ipow(Integer(17), Integer(1))
		* ipow(Integer(23), Integer(1))
		* ipow(Integer(29), Integer(1))
		* ipow(Integer(31), Integer(1))
		* ipow(Integer(37), Integer(1));
	primefactors<nbits, uint32_t> factors;
	primeFactorization(a, factors);
	for (size_t i = 0; i < factors.size(); ++i) {
		cout << " factor " << factors[i].first << " exponent " << factors[i].second << endl;
	}

#ifdef NOW
	// primeFactorization doesn't seem to work for a prime number
	factors.clear();
	// find all prime factors of a number
	a = ipow(Integer(2), Integer(5))
		* ipow(Integer(3), Integer(4))
		* ipow(Integer(5), Integer(3))
		* ipow(Integer(7), Integer(2))
		* ipow(Integer(11), Integer(1))
		* ipow(Integer(13), Integer(1))
		* ipow(Integer(17), Integer(1))
		* ipow(Integer(23), Integer(1))
		* ipow(Integer(29), Integer(1))
		* ipow(Integer(31), Integer(1))
		* ipow(Integer(37), Integer(1)) + 1;
	primeFactorization(a, factors);
	for (size_t i = 0; i < factors.size(); ++i) {
		cout << " factor " << factors[i].first << " exponent " << factors[i].second << endl;
	}
#endif

#else // MANUAL_TESTING

	// GCD of three numbers is
	// gcd(a, b, c) == gcd(a, gcd(b, c)) == gcd(gcd(a, b), c) == gcd(b, gcd(a, c))


#if STRESS_TESTING

#endif // STRESS_TESTING
#endif // MANUAL_TESTING


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
