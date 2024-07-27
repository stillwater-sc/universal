// large_lcm.cpp: calculating a least common multiple of a very large set
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <typeinfo>
#include <chrono>
// include the number system we want to use, and configure overflow exceptions so we can capture failures
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>

template<unsigned nbits, typename BlockType>
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

// calculate the Least Common Multiple of a set of N random values 
template<unsigned nbits>
void calculateLCM(unsigned N, const char* dumpFile = "default_dump_file.txt") 	// this triggers the integer_overflow exception
{
	using Integer = sw::universal::integer<nbits, uint32_t>;
	Integer factor;

	std::random_device rd{};
	std::mt19937 engine{ rd() };
	std::uniform_real_distribution<double> dist{0.0, 10000.0 };

	std::vector<Integer> v;
	for (unsigned i = 0; i < N; ++i) {
		factor = dist(engine);
		if (factor.iseven()) ++factor;
		v.push_back(factor);
	}
	try {
		MeasureLCM(v);
	}
	catch (const sw::universal::integer_overflow& e) {
		std::cerr << e.what() << '\n';
		std::cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
		std::ofstream out;
		out.open(dumpFile);
		for (size_t i = 0; i < v.size(); ++i) {
			out << v[i] << '\n';
		}
		out.close();
	}
}
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;
	   	
	calculateLCM<512>(10, "lcm_dataset_1.txt");
	calculateLCM<1024>(100, "lcm_dataset_2.txt");

#if STRESS_TESTING
	calculateLCM<2048>(1000, "lcm_dataset_3.txt");
#endif

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
