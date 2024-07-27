// primes.cpp: prime number finding tests on arbitrary fixed-size integers
//
// prime number generation and Fermat factorization
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <numeric>
#include <chrono>
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/primes.hpp>
#include <universal/verification/test_suite.hpp>

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

/*
.\math_primes.exe
gcd of 1024 and 512 = 512
gcd of 1024 and 512 = 512

Find all prime numbers in a range
1 prime numbers in range [9223372036854775680, 9223372036854775807)
largest prime: 9223372036854775783 is 19 decades
 9223372036854775783
5.9541sec
3 prime numbers in range [10376293541461622656, 10376293541461622783)
largest prime: 10376293541461622777 is 20 decades
 10376293541461622659 10376293541461622771 10376293541461622777
21.0991sec
0 prime numbers in range [11529215046068469632, 11529215046068469759)
5.50648sec
4 prime numbers in range [12682136550675316608, 12682136550675316735)
largest prime: 12682136550675316723 is 20 decades
 12682136550675316609 12682136550675316691 12682136550675316717 12682136550675316723
27.7437sec
3 prime numbers in range [13835058055282163584, 13835058055282163711)
largest prime: 13835058055282163681 is 20 decades
 13835058055282163621 13835058055282163641 13835058055282163681
30.5823sec
2 prime numbers in range [14987979559889010560, 14987979559889010687)
largest prime: 14987979559889010641 is 20 decades
 14987979559889010581 14987979559889010641
14.1723sec
4 prime numbers in range [16140901064495857536, 16140901064495857663)
largest prime: 16140901064495857651 is 20 decades
 16140901064495857577 16140901064495857597 16140901064495857613 16140901064495857651
 28.8434sec
*/
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
		0xDFFF'FFFF'FFFF'FF80
//		0xFFFF'FFFF'FFFF'FF00
	};
	Integer b[] = {
		0x7FFF'FFFF'FFFF'FFFF,
		0x8FFF'FFFF'FFFF'FFFF,
		0x9FFF'FFFF'FFFF'FFFF,
		0xAFFF'FFFF'FFFF'FFFF,
		0xBFFF'FFFF'FFFF'FFFF,
		0xCFFF'FFFF'FFFF'FFFF,
		0xDFFF'FFFF'FFFF'FFFF
//		0xFFFF'FFFF'FFFF'FFFF  // this yields an infinite loop
	};
	for (size_t i = 0; i < 7; ++i) {
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

	std::string test_suite  = "Prime generation for integer<>";
	std::string test_tag    = "integer<> primes";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	constexpr size_t nbits = 40;
	using BlockType = std::uint32_t;
	using Integer = integer<nbits, BlockType>;

#if MANUAL_TESTING

	Integer a, b;
	std::vector<Integer> v;

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

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	long l1 = 1024l;
	long l2 = 512;
	std::cout << "gcd of " << l1 << " and " << l2 << " = " << sw::universal::gcd(l1, l2) << '\n';
	std::cout << "gcd of " << l1 << " and " << l2 << " = " << std::gcd(l1, l2) << '\n';

	Integer a(l1), b(l2);
	std::cout << "gcd of " << a  << " and " << b  << " = " << sw::universal::gcd(a, b) << '\n';
#endif

#if REGRESSION_LEVEL_2
	std::cout << "\nFind all prime numbers in a range\n";
	{
		Integer a, b;
		std::vector<Integer> v;
		a = 2; b = 1000;
		primeNumbersInRange(a, b, v);
		std::cout << v.size() << " prime numbers in range [" << a << ", " << b << ")\n";
		printPrimes(v);
	}

	std::cout << "\nFind all prime factors of the number : ";
	{
		Integer a;
		// find all prime factors of a number
		a = ipow(Integer(2), Integer(5))
			* ipow(Integer(3), Integer(4))
			* ipow(Integer(5), Integer(3))
			* ipow(Integer(7), Integer(2))
			* ipow(Integer(13), Integer(1))
			* ipow(Integer(37), Integer(1));
		std::cout << to_binary(a) << " : " << a << '\n';
		primefactors<Integer> factors;
		primeFactorization(a, factors);
		for (size_t i = 0; i < factors.size(); ++i) {
			std::cout << " factor " << factors[i].first << " exponent " << factors[i].second << '\n';
		}
	}
#endif

#if REGRESSION_LEVEL_3
	{
		constexpr Integer a(SpecificValue::maxpos);
		std::cout << "maxpos for " << type_tag(a) << " = " << a << '\n' << to_binary(a) << '\n';
	}
#endif

#if REGRESSION_LEVEL_4
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
#endif





	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
