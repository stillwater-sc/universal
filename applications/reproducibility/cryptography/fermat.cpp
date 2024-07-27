// fermat.cpp: factor numbers using Fermat's basic factorization algorithm, a^2 - b^2 = N
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <random>
#include <typeinfo>
#include <chrono>
// include the number system we want to use, and configure overflow exceptions so we can capture failures
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/primes.hpp>

template<size_t nbits, typename BlockType>
bool miller_rabin(const sw::universal::integer<nbits, BlockType>& a, int reps) {
	return true;
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using namespace std::chrono;

	{
		constexpr size_t nbits = 1024;
		using Integer = integer<nbits, uint32_t, IntegerNumberType::IntegerNumber>;
		// some primes to try
		Integer a = 53;
//		Integer a = 1049;
//		Integer a = 9973;
//		Integer a = 99991;
//		Integer a = 101737;
//		Integer a = 999983;

//		Integer factor;

		std::random_device rd{};
		std::mt19937 engine{ rd() };
		std::uniform_real_distribution<long double> dist{0.0, 1000000.0 };

//		Prime factorization of 999983
//			2.69353sec
//			factor 999983 exponent 1
		std::cout << "\nPrime factorization of " << a << '\n';
		{
			primefactors<Integer> factors;
			steady_clock::time_point begin = steady_clock::now();
			primeFactorization(a, factors);
			steady_clock::time_point end = steady_clock::now();
			duration<double> time_span = duration_cast<duration<double>> (end - begin);
			double elapsed_time = time_span.count();

			std::cout << elapsed_time << "sec\n";

			for (size_t i = 0; i < factors.size(); ++i) {
				std::cout << " factor " << factors[i].first << " exponent " << factors[i].second << '\n';
			}
		}

		/*
		Prime factorization of 53
			0.0044623sec
			 factor 53 exponent 1

		Fermat's factorization: to demonstrate it is much slower
			factor 53 exponent 1
			0.164085sec

		Prime factorization of 9973
			0.142941sec
			factor 9973 exponent 1

		Fermat's factorization: to demonstrate it is much slower
			factor 9973 exponent 1
			29.0085sec
		*/

		// test Fermat's method
		std::cout << "\nFermat's factorization: to demonstrate it is much slower\n";
		try {
			std::stack<Integer> factors;
			factors.push(a);

			steady_clock::time_point begin = steady_clock::now();
			while (!factors.empty()) {
				Integer factor = factors.top();
				factors.pop();
				/*
				if (miller_rabin(factor, 25)) {
					// factor is prime
					cout << factor << endl;
					continue;
				}
				*/
				Integer result = fermatFactorization(factor);
				if (result == 1) {
					std::cout << "factor " << factor << " exponent " << result << '\n';
					continue;
				}
				factors.push(result);
				factors.push(factor / result);
			}

			steady_clock::time_point end = steady_clock::now();
			duration<double> time_span = duration_cast<duration<double>> (end - begin);
			double elapsed_time = time_span.count();

			std::cout << elapsed_time << "sec\n";
		}
		catch (const integer_overflow& e) {
			std::cerr << e.what() << '\n';
			std::cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
		}

	}

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
