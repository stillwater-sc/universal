// primes.cpp: prime finding tests
//
// prime number generation and Fermat factorization
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/math_functions.hpp>
#include <universal/number/integer/primes.hpp>

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0
#define ELABORATE_TEST 0

int main() 
try {
	using namespace sw::universal;
	constexpr size_t nbits = 1024;
	using BlockType = uint32_t;
	using Integer = integer<nbits, BlockType>;
	Integer a, b, c;
	std::vector<Integer> v;

#if MANUAL_TESTING

	std::cout << "\nFind all prime numbers in a range\n";
	v.clear();
	a = 2; b = 100;
	primeNumbersInRange(a, b, v);
	std::cout << v.size() << " prime numbers in range [" << a << ", " << b << ")\n";

	std::cout << "\nCheck primeness of a couple of values around 1k\n";
	a = 1024 + 1;
	for ( a = 1025; a < 1050; a += 2) { // skip the even numbers
		std::cout << a << (isPrime(a) ? " is a prime number\n" : " is not a prime number\n");
	}

#if STRESS_TESTING
	// this is relatively slow
	{
		// test Fermat's method
		std::cout << "\nFermat's factorization\n";
		a = 1024;
		for (Integer i = a + 1; i < a + 25; i += 2) {
			std::cout << i << " " << fermatFactorization(i) << '\n';
		}
	}
#endif

#if ELABORATE
	// primeFactorization requires significant runtime when factorization is sparse
	//  time ./math_primes.exe
	// 
	// Find all prime factors of the number : 29526726473244001
	//	factor 199 exponent 1
	//	factor 281 exponent 1
	//	factor 63281 exponent 1
	//	factor 8344159 exponent 1
	//
	//	real    103m27.272s
	//	user    0m0.000s
	//	sys     0m0.031s
	std::cout << "\nFind all prime factors of the number : ";
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
	std::cout << a << '\n';
	primefactors<nbits, uint32_t> factors;
	primeFactorization(a, factors);
	for (size_t i = 0; i < factors.size(); ++i) {
		std::cout << " factor " << factors[i].first << " exponent " << factors[i].second << '\n';
	}
#endif

#else // MANUAL_TESTING

	std::cout << "\nFind all prime numbers in a range\n";
	v.clear();
	a = 2; b = 100;
	primeNumbersInRange(a, b, v);
	std::cout << v.size() << " prime numbers in range [" << a << ", " << b << ")\n";

	// GCD of three numbers is
	// gcd(a, b, c) == gcd(a, gcd(b, c)) == gcd(gcd(a, b), c) == gcd(b, gcd(a, c))

	{
		std::cout << "\nFind all prime factors of the number : ";
		// find all prime factors of a number
		a = ipow(Integer(2), Integer(5))
			* ipow(Integer(3), Integer(4))
			* ipow(Integer(5), Integer(3))
			* ipow(Integer(7), Integer(2))
			* ipow(Integer(13), Integer(1))
			* ipow(Integer(37), Integer(1));
		std::cout << a << '\n';
		primefactors<nbits, uint32_t> factors;
		primeFactorization(a, factors);
		for (size_t i = 0; i < factors.size(); ++i) {
			std::cout << " factor " << factors[i].first << " exponent " << factors[i].second << '\n';
		}
	}

#if STRESS_TESTING
	{
		std::cout << "\nFind all prime factors of a number\n";
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
			std::cout << " factor " << factors[i].first << " exponent " << factors[i].second << '\n';
		}
	}

#endif // STRESS_TESTING

#endif // MANUAL_TESTING


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
