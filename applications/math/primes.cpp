// primes.cpp: prime finding tests
//
// prime number generation and Fermat factorization
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <numeric>
#include <chrono>
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/math_functions.hpp>
#include <universal/number/integer/primes.hpp>

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0
#define ELABORATE_TEST 0

	// 1 prime numbers in range [9223372036854775776, 9223372036854775807)
	// largest prime : 9223372036854775783 is 19 decades
	//	9223372036854775783
	//	4.93456sec


template<typename Integer>
void generatePrimes(Integer a, Integer b)
{
	std::vector<Integer> v;
	sw::universal::primeNumbersInRange<Integer>(a, b, v);
	std::cout << v.size() << " prime numbers in range [" << a << ", " << b << ")\n";
	sw::universal::printPrimes(v);
}

template<typename Integer = uint64_t>
void MeasureElapsedTimeOfPrimeGeneration()
{
	using namespace std::chrono;
	Integer a[] = {
		0x7FFF'FFFF'FFFF'FF80,
		0x8FFF'FFFF'FFFF'FF80,
		0x9FFF'FFFF'FFFF'FF80,
		0xAFFF'FFFF'FFFF'FF80,
		0xBFFF'FFFF'FFFF'FF80,
		0xCFFF'FFFF'FFFF'FF80,
		0xDFFF'FFFF'FFFF'FF80,
		0xFFFF'FFFF'FFFF'FF80
	};
	Integer b[] = {
		0x7FFF'FFFF'FFFF'FFFF,
		0x8FFF'FFFF'FFFF'FFFF,
		0x9FFF'FFFF'FFFF'FFFF,
		0xAFFF'FFFF'FFFF'FFFF,
		0xBFFF'FFFF'FFFF'FFFF,
		0xCFFF'FFFF'FFFF'FFFF,
		0xDFFF'FFFF'FFFF'FFFF,
		0xFFFF'FFFF'FFFF'FFFF
	};
	for (size_t i = 0; i < 8; ++i) {
		steady_clock::time_point begin = steady_clock::now();
		generatePrimes(a[i], b[i]);
		steady_clock::time_point end = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>> (end - begin);
		double elapsed_time = time_span.count();

		std::cout << elapsed_time << "sec\n";
	}
}

int main() 
try {
	using namespace sw::universal;
	constexpr size_t nbits = 32;
	using BlockType = uint32_t;
	using Integer = integer<nbits, BlockType>;

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

	// this takes a couple of minutes

	MeasureElapsedTimeOfPrimeGeneration();
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

	long l1 = 1024l;
	long l2 = 512;
	std::cout << "gcd of " << l1 << " and " << l2 << " = " << sw::universal::gcd(l1, l2) << '\n';
	std::cout << "gcd of " << l1 << " and " << l2 << " = " << std::gcd(l1, l2) << '\n';

	std::cout << "\nFind all prime numbers in a range\n";
	{
		Integer a, b;
		std::vector<Integer> v;
		a = 2; b = 1000;
		primeNumbersInRange(a, b, v);
		std::cout << v.size() << " prime numbers in range [" << a << ", " << b << ")\n";
		printPrimes(v);
	}

	// GCD of three numbers is
	// gcd(a, b, c) == gcd(a, gcd(b, c)) == gcd(gcd(a, b), c) == gcd(b, gcd(a, c))

	std::cout << "\nFind all prime factors of the number : ";
	{
		Integer a, b;
		// find all prime factors of a number
		a = ipow(Integer(2), Integer(5))
			* ipow(Integer(3), Integer(4))
			* ipow(Integer(5), Integer(3))
			* ipow(Integer(7), Integer(2))
			* ipow(Integer(13), Integer(1))
			* ipow(Integer(37), Integer(1));
		std::cout << a << '\n';
		primefactors<Integer> factors;
		primeFactorization(a, factors);
		for (size_t i = 0; i < factors.size(); ++i) {
			std::cout << " factor " << factors[i].first << " exponent " << factors[i].second << '\n';
		}
	}

	{
		constexpr Integer a(SpecificValue::maxpos);
		std::cout << "maxpos for " << type_tag(a) << " = " << a << '\n' << to_binary(a) << '\n';
	}

#if STRESS_TESTING
	std::cout << "\nFind all prime factors of a number\n";
	{
		Integer a;
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
		primefactors<Integer> factors;
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
