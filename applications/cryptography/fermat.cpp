// fermat.cpp: factor numbers using Fermat's basic factorization algorithm, a^2 - b^2 = N
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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

	{
		constexpr size_t nbits = 1024;
		using Integer = integer<nbits, uint32_t>;
		// some primes to try
		Integer a = 53;
//		Integer a = 1049;
//		Integer a = 9973;
//		Integer a = 99991;
//		Integer a = 101737;
//		Integer a = 999983;

		Integer factor;

		std::random_device rd{};
		std::mt19937 engine{ rd() };
		std::uniform_real_distribution<long double> dist{0.0, 1000000.0 };

		{
			primefactors<nbits, uint32_t> factors;
			primeFactorization(a, factors);
			for (size_t i = 0; i < factors.size(); ++i) {
				std::cout << " factor " << factors[i].first << " exponent " << factors[i].second << '\n';
			}
		}

		// test Fermat's method
		std::cout << "\nFermat's factorization: to demonstrate it is much slower\n";
		try {
			std::stack<Integer> factors;
			factors.push(a);

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

		}
		catch (const integer_overflow& e) {
			std::cerr << e.what() << '\n';
			std::cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
		}

	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
