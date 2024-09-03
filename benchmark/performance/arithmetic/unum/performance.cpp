// performance.cpp : performance benchmarking for adaptive precision universal numbers (unum Type 1)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
// configure the arithmetic class
#define UNUM_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/unum/unum.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

/*
   The goal of the adaptive precision unums is to provide a constrained big real type
   that enables fast computation with higher-precision than native so that the type
   can be used for forward error analysis studies.
*/

template<typename Scalar>
void AdditionSubtractionWorkload(uint64_t NR_OPS) {
	Scalar a, b, c, d;
	a = b = c = d = 0xFFFFFFFFFFFFFFFF;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a + b;
		a = c - b;
	}
}

template<typename Scalar>
void MultiplicationWorkload(uint64_t NR_OPS) {
	Scalar a, b, c, d;
	a = b = c = d = 0xFFFFFFFFFFFFFFFF;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a * b;
		c.clear(); // reset to zero so d = c is fast
		d = c;
	}
}

template<typename Scalar>
void DivisionWorkload(uint64_t NR_OPS) {
	Scalar a, b, c, d;
	a = b = c = d = 0xFFFFFFFFFFFFFFFF;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a / b;
		c.clear(); // reset to zero so d = c is fast
		d = c;
	}
}

void TestArithmeticOperatorPerformance() {
	std::cout << "\nUNUM Arithmetic operator performance\n";

#if 0
	uint64_t NR_OPS = 1000000;

	PerformanceRunner("unum<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::unum<16> >, NR_OPS);
	PerformanceRunner("unum<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::unum<32> >, NR_OPS);
	PerformanceRunner("unum<64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::unum<64> >, NR_OPS);
	PerformanceRunner("unum<128>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::unum<128> >, NR_OPS / 2);
	PerformanceRunner("unum<256>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::unum<256> >, NR_OPS / 4);
	PerformanceRunner("unum<512>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::unum<512> >, NR_OPS / 8);
	PerformanceRunner("unum<1024> add/subtract  ", AdditionSubtractionWorkload< sw::universal::unum<1024> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("unum<16>   division      ", DivisionWorkload< sw::universal::unum<16> >, NR_OPS);
	PerformanceRunner("unum<32>   division      ", DivisionWorkload< sw::universal::unum<32> >, NR_OPS);
	PerformanceRunner("unum<64>   division      ", DivisionWorkload< sw::universal::unum<64> >, NR_OPS / 2);
	PerformanceRunner("unum<128>  division      ", DivisionWorkload< sw::universal::unum<128> >, NR_OPS / 4);
	PerformanceRunner("unum<512>  division      ", DivisionWorkload< sw::universal::unum<512> >, NR_OPS / 8);
	PerformanceRunner("unum<1024> division      ", DivisionWorkload< sw::universal::unum<1024> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("unum<16>   remainder     ", RemainderWorkload< sw::universal::unum<16> >, NR_OPS);
	PerformanceRunner("unum<32>   remainder     ", RemainderWorkload< sw::universal::unum<32> >, NR_OPS);
	PerformanceRunner("unum<64>   remainder     ", RemainderWorkload< sw::universal::unum<64> >, NR_OPS / 2);
	PerformanceRunner("unum<128>  remainder     ", RemainderWorkload< sw::universal::unum<128> >, NR_OPS / 4);
	PerformanceRunner("unum<512>  remainder     ", RemainderWorkload< sw::universal::unum<512> >, NR_OPS / 8);
	PerformanceRunner("unum<1024> remainder     ", RemainderWorkload< sw::universal::unum<1024> >, NR_OPS / 16);

	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("unum<16>   multiplication", MultiplicationWorkload< sw::universal::unum<16> >, NR_OPS);
	PerformanceRunner("unum<32>   multiplication", MultiplicationWorkload< sw::universal::unum<32> >, NR_OPS / 2);
	PerformanceRunner("unum<64>   multiplication", MultiplicationWorkload< sw::universal::unum<64> >, NR_OPS / 4);
	PerformanceRunner("unum<128>  multiplication", MultiplicationWorkload< sw::universal::unum<128> >, NR_OPS / 8);
	PerformanceRunner("unum<512>  multiplication", MultiplicationWorkload< sw::universal::unum<512> >, NR_OPS / 16);
	PerformanceRunner("unum<1024> multiplication", MultiplicationWorkload< sw::universal::unum<1024> >, NR_OPS / 32);
#endif
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "Integer operator performance benchmarking";

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
