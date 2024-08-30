// performance.cpp : performance benchmarking for abitrary fixed-precision valids
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <chrono>
// configure the arithmetic class
#define VALID_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/valid/valid.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nVALID Arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("valid<8,2>    add/subtract  ", AdditionSubtractionWorkload< sw::universal::valid<8,2> >, NR_OPS);
	PerformanceRunner("valid<16,2>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::valid<16,2> >, NR_OPS);
	PerformanceRunner("valid<32,2>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::valid<32,2> >, NR_OPS);
	PerformanceRunner("valid<64,2>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::valid<64,2> >, NR_OPS);
	PerformanceRunner("valid<128,2>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::valid<128,2> >, NR_OPS / 2);
	PerformanceRunner("valid<256,2>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::valid<256,2> >, NR_OPS / 4);

	NR_OPS = 1024 * 32;
	PerformanceRunner("valid<8,2>    division      ", DivisionWorkload< sw::universal::valid<8,2> >, NR_OPS);
	PerformanceRunner("valid<16,2>   division      ", DivisionWorkload< sw::universal::valid<16,2> >, NR_OPS);
	PerformanceRunner("valid<32,2>   division      ", DivisionWorkload< sw::universal::valid<32,2> >, NR_OPS);
	PerformanceRunner("valid<64,2>   division      ", DivisionWorkload< sw::universal::valid<64,2> >, NR_OPS / 2);
	PerformanceRunner("valid<128,2>  division      ", DivisionWorkload< sw::universal::valid<128,2> >, NR_OPS / 4);
	PerformanceRunner("valid<256,2>  division      ", DivisionWorkload< sw::universal::valid<256,2> >, NR_OPS / 4);

	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("valid<8,2>    multiplication", MultiplicationWorkload< sw::universal::valid<8,2> >, NR_OPS);
	PerformanceRunner("valid<16,2>   multiplication", MultiplicationWorkload< sw::universal::valid<16,2> >, NR_OPS);
	PerformanceRunner("valid<32,2>   multiplication", MultiplicationWorkload< sw::universal::valid<32,2> >, NR_OPS / 2);
	PerformanceRunner("valid<64,2>   multiplication", MultiplicationWorkload< sw::universal::valid<64,2> >, NR_OPS / 4);
	PerformanceRunner("valid<128,2>  multiplication", MultiplicationWorkload< sw::universal::valid<128,2> >, NR_OPS / 8);
	PerformanceRunner("valid<256,2>  multiplication", MultiplicationWorkload< sw::universal::valid<256,2> >, NR_OPS / 8);
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "VALID operator performance benchmarking";

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
