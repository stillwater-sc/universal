// performance.cpp : performance benchmarking for abitrary fixed-precision logarithmic numbers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <chrono>
// configure the arithmetic class
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/lns/lns>
// is representable
#include <universal/functions/isrepresentable.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/performance_runner.hpp>

/*
   The goal of logarithmic numbers is to provide a number system representation
   for applications that manipulate exponential properties.
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
	using namespace std;
	using namespace sw::universal;
	cout << endl << "Logarithmic LNS Arithmetic operator performance" << endl;

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("lns<8>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<16> >, NR_OPS);
	PerformanceRunner("lns<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<16> >, NR_OPS);
	PerformanceRunner("lns<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<32> >, NR_OPS);
	PerformanceRunner("lns<64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<64> >, NR_OPS);
	PerformanceRunner("lns<128>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<128> >, NR_OPS / 2);
	PerformanceRunner("lns<256>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<256> >, NR_OPS / 4);
	PerformanceRunner("lns<512>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<512> >, NR_OPS / 8);
	PerformanceRunner("lns<1024> add/subtract  ", AdditionSubtractionWorkload< sw::universal::lns<1024> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("lns<16>   division      ", DivisionWorkload< sw::universal::lns<16> >, NR_OPS);
	PerformanceRunner("lns<32>   division      ", DivisionWorkload< sw::universal::lns<32> >, NR_OPS);
	PerformanceRunner("lns<64>   division      ", DivisionWorkload< sw::universal::lns<64> >, NR_OPS / 2);
	PerformanceRunner("lns<128>  division      ", DivisionWorkload< sw::universal::lns<128> >, NR_OPS / 4);
	PerformanceRunner("lns<512>  division      ", DivisionWorkload< sw::universal::lns<512> >, NR_OPS / 8);
	PerformanceRunner("lns<1024> division      ", DivisionWorkload< sw::universal::lns<1024> >, NR_OPS / 16);

	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("lns<16>   multiplication", MultiplicationWorkload< sw::universal::lns<16> >, NR_OPS);
	PerformanceRunner("lns<32>   multiplication", MultiplicationWorkload< sw::universal::lns<32> >, NR_OPS / 2);
	PerformanceRunner("lns<64>   multiplication", MultiplicationWorkload< sw::universal::lns<64> >, NR_OPS / 4);
	PerformanceRunner("lns<128>  multiplication", MultiplicationWorkload< sw::universal::lns<128> >, NR_OPS / 8);
	PerformanceRunner("lns<512>  multiplication", MultiplicationWorkload< sw::universal::lns<512> >, NR_OPS / 16);
	PerformanceRunner("lns<1024> multiplication", MultiplicationWorkload< sw::universal::lns<1024> >, NR_OPS / 32);
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "LNS logarithmic operator performance benchmarking";

#if MANUAL_TESTING

	TestArithmeticOperatorPerformance();

	cout << "done" << endl;

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
	std::cerr << msg << '\n';
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
