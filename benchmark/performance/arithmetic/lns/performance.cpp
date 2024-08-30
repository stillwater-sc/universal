// performance.cpp : performance benchmarking for abitrary fixed-precision logarithmic numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <chrono>
// configure the arithmetic class
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

/*
   The goal of logarithmic numbers is to provide a number system representation
   for applications that manipulate exponential properties.
*/

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nLogarithmic LNS Arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("lns<8>    add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<16, 5> >, NR_OPS);
	PerformanceRunner("lns<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<16, 5> >, NR_OPS);
	PerformanceRunner("lns<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<32, 8> >, NR_OPS);
//	PerformanceRunner("lns<64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<64, 11> >, NR_OPS);

	NR_OPS = 1024 * 32;
	PerformanceRunner("lns<16>   division      ", DivisionWorkload< sw::universal::lns<16, 5> >, NR_OPS);
	PerformanceRunner("lns<32>   division      ", DivisionWorkload< sw::universal::lns<32, 8> >, NR_OPS);
//	PerformanceRunner("lns<64>   division      ", DivisionWorkload< sw::universal::lns<64, 11> >, NR_OPS / 2);


	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("lns<16>   multiplication", MultiplicationWorkload< sw::universal::lns<16, 5> >, NR_OPS);
	PerformanceRunner("lns<32>   multiplication", MultiplicationWorkload< sw::universal::lns<32, 8> >, NR_OPS / 2);
//	PerformanceRunner("lns<64>   multiplication", MultiplicationWorkload< sw::universal::lns<64, 11> >, NR_OPS / 4);

}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "LNS logarithmic operator performance benchmarking";

#if MANUAL_TESTING

	TestArithmeticOperatorPerformance();

	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestArithmeticOperatorPerformance();

#if STRESS_TESTING

#endif // STRESS_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}

/*
ETLO
Date run : 2/23/2020
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/
