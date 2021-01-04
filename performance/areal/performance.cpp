// performance.cpp : performance benchmarking for abitrary fixed-precision reals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <chrono>
// configure the areal arithmetic class
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/areal/areal>
// is representable
#include <universal/functions/isrepresentable.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/performance_runner.hpp>

/*
   The goal of the arbitrary fixed-precision reals is to provide a constrained 
   linear floating-point type to explore the benefits of multi-precision algorithms.
*/

template<typename RealType>
void AdditionSubtractionWorkload(uint64_t NR_OPS) {
	RealType a, b, c, d;
	a = b = c = d = 0xFFFFFFFFFFFFFFFF;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a + b;
		a = c - b;
	}
}

template<typename RealType>
void MultiplicationWorkload(uint64_t NR_OPS) {
	RealType a, b, c, d;
	a = b = c = d = 0xFFFFFFFFFFFFFFFF;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a * b;
		c.clear(); // reset to zero so d = c is fast
		d = c;
	}
}

template<typename RealType>
void DivisionWorkload(uint64_t NR_OPS) {
	RealType a, b, c, d;
	a = b = c = d = 0xFFFFFFFFFFFFFFFF;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a / b;
		c.clear(); // reset to zero so d = c is fast
		d = c;
	}
}

template<typename RealType>
void RemainderWorkload(uint64_t NR_OPS) {
	RealType a, b, c, d;
	a = b = c = d = 0xFFFFFFFFFFFFFFFF;
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a % b;
		c.clear(); // reset to zero so d = c is fast
		d = c;
	}
}

void TestArithmeticOperatorPerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "AREAL Arithmetic operator performance" << endl;

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("areal<8,2,uint8_t>      add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<8,2,uint8_t> >, NR_OPS);
	PerformanceRunner("areal<16,5,uint16_t>    add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<32,8,uint32_t>    add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<32,8,uint32_t> >, NR_OPS);
	PerformanceRunner("areal<64,11,uint64_t>   add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<64,11,uint64_t> >, NR_OPS);
//	PerformanceRunner("areal<128,15,uint64_t>  add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("areal<256,15,uint64_t   add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("areal<512,15,uint64_t>  add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("areal<1024,15,uint64_t> add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<1024,15,uint64_t> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("areal<8,2,uint16_t>     division       ", DivisionWorkload< sw::universal::areal<8,2,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<16,5,uint16_t>    division       ", DivisionWorkload< sw::universal::areal<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<32,8,uint32_t>    division       ", DivisionWorkload< sw::universal::areal<32,8,uint32_t> >, NR_OPS);
	PerformanceRunner("areal<64,11,uint64_t>   division       ", DivisionWorkload< sw::universal::areal<64,11,uint64_t> >, NR_OPS);
//	PerformanceRunner("areal<128,15,uint64_t>  division       ", DivisionWorkload< sw::universal::areal<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("areal<256,15,uint64_t   division       ", DivisionWorkload< sw::universal::areal<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("areal<512,15,uint64_t>  division       ", DivisionWorkload< sw::universal::areal<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("areal<1024,15,uint64_t> division       ", DivisionWorkload< sw::universal::areal<1024,15,uint64_t> >, NR_OPS / 16);

	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("areal<8,2,uint16_t>     multiplication ", MultiplicationWorkload< sw::universal::areal<8,2,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<16,5,uint16_t>    multiplication ", MultiplicationWorkload< sw::universal::areal<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<32,8,uint32_t>    multiplication ", MultiplicationWorkload< sw::universal::areal<32,8,uint32_t> >, NR_OPS);
	PerformanceRunner("areal<64,11,uint64_t>   multiplication ", MultiplicationWorkload< sw::universal::areal<64,11,uint64_t> >, NR_OPS);
//	PerformanceRunner("areal<128,15,uint64_t>  multiplication ", MultiplicationWorkload< sw::universal::areal<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("areal<256,15,uint64_t   multiplication ", MultiplicationWorkload< sw::universal::areal<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("areal<512,15,uint64_t>  multiplication ", MultiplicationWorkload< sw::universal::areal<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("areal<1024,15,uint64_t> multiplication ", MultiplicationWorkload< sw::universal::areal<1024,15,uint64_t> >, NR_OPS / 16);

}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "AREAL operator performance benchmarking";

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
