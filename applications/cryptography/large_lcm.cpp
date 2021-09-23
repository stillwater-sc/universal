// large_lcm.cpp: calculating a least common multiple of a very large set
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <typeinfo>
#include <chrono>
// include the number system we want to use, and configure overflow exceptions so we can capture failures
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>

template<size_t nbits, typename BlockType>
void MeasureLCM(const std::vector<sw::universal::integer<nbits, BlockType>>& v) {
	std::chrono::steady_clock::time_point begin, end;
	begin = std::chrono::steady_clock::now();
	using Integer = sw::universal::integer<nbits, BlockType>;
	Integer least_common_multple = lcm(v);
	end = std::chrono::steady_clock::now();
	using TimeReal = float;
	std::chrono::duration<TimeReal> time_span = std::chrono::duration_cast<std::chrono::duration<TimeReal >> (end - begin);
	TimeReal elapsed = time_span.count();

	std::cout << "In " << elapsed << " seconds calculated LCM of " << v.size() << " elements of type " << typeid(Integer).name()
		<< " to be\n" << least_common_multple << '\n';
}

#define STRESS_TESTING 0

// Warning	C6262	Function uses '16448' bytes of stack : exceeds / analyze : stacksize '16384'.Consider moving some data to heap.crypto_large_lcm	C : \Users\tomtz\Documents\dev\clones\universal\applications\cryptography\large_lcm.cpp	31
// TODO: what gets allocated on the stack? Integer factor, but that is max 256bytes
int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	   	
	{
		constexpr size_t nbits = 512;
		using Integer = integer<nbits, uint32_t>;
		Integer factor;

		// use random_device to generate a seed for Mersenne twister engine
		std::random_device rd{};
		std::mt19937 engine{ rd() };
		std::uniform_real_distribution<long double> dist{0.0, 1000000000000.0l };

		std::vector<Integer> v;
		for (int i = 0; i < 10; ++i) {
			factor = dist(engine);
			if (factor.iseven()) ++factor;
			// cout << factor << endl;
			v.push_back(factor);
		}
		try {
			MeasureLCM(v);
		}
		catch (const integer_overflow& e) {
			std::cerr << e.what() << '\n';
			std::cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
			std::ofstream out;
			out.open("lcm_dataset_1.txt");
			for (size_t i = 0; i < v.size(); ++i) {
				out << v[i] << '\n';
			}
			out.close();
		}
	}

	// this triggers the integer_overflow exception
	{
		constexpr size_t nbits = 1024;
		using Integer = integer<nbits, uint32_t>;
		Integer factor;

		std::random_device rd{};
		std::mt19937 engine{ rd() };
		std::uniform_real_distribution<long double> dist{0.0, 100000.0 };

		std::vector<Integer> v;
		for (int i = 0; i < 100; ++i) {
			factor = dist(engine);
			if (factor.iseven()) ++factor;
			v.push_back(factor);
		}
		try {
			MeasureLCM(v);
		}
		catch (const integer_overflow& e) {
			std::cerr << e.what() << '\n';
			std::cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
			std::ofstream out;
			out.open("lcm_dataset_2.txt");
			for (size_t i = 0; i < v.size(); ++i) {
				out << v[i] << '\n';
			}
			out.close();
		}
	}

#if STRESS_TESTING
	{
		constexpr size_t nbits = 2048;
		using Integer = integer<nbits, uint32_t>;
		Integer factor;

		std::random_device rd{};
		std::mt19937 engine{ rd() };
		std::uniform_real_distribution<long double> dist{0.0, 1000.0 };

		std::vector<Integer> v;
		for (int i = 0; i < 1000; ++i) {
			factor = dist(engine);
			if (factor.iseven()) ++factor;
			v.push_back(factor);
		}
		try {
			MeasureLCM(v);
		}
		catch (const integer_overflow& e) {
			std::cerr << e.what() << '\n';
			std::cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
			std::ofstream out;
			out.open("lcm_dataset_3.txt");
			for (size_t i = 0; i < v.size(); ++i) {
				out << v[i] << '\n';
			}
			out.close();
		}
	}
#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
