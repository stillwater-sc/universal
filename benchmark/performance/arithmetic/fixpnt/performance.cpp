//  performance.cpp : performance benchmarking for fixed-sized, arbitrary precision fixpnts
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <chrono>
// configure the fixpnt arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

/*
   The goal of the arbitrary fixpnts is to provide a constrained big fixpnt type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

// test performance of shift operator on fixpnt<> class
void TestShiftOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nFIXPNT Fixed-Point Logical shift operator performance\n";

	constexpr uint64_t NR_OPS = 1000000;

	PerformanceRunner("fixpnt<   8,  4, Saturate, uint8_t>  shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt<   8,  4, Saturate, uint8_t> >, NR_OPS);
	PerformanceRunner("fixpnt<  16,  8, Saturate, uint16_t> shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt<  16,  8, Saturate, uint16_t> >, NR_OPS);
	PerformanceRunner("fixpnt<  32, 16, Saturate, uint32_t> shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt<  32, 16, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt<  64, 32, Saturate, uint32_t> shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt<  64, 32, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 128, 32, Saturate, uint32_t> shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt< 128, 32, Saturate, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("fixpnt< 256, 32, Saturate, uint32_t> shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt< 256, 32, Saturate, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("fixpnt< 512, 32, Saturate, uint32_t> shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt< 512, 32, Saturate, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("fixpnt<1024, 32, Saturate, uint32_t> shifts         ", ShiftPerformanceWorkload< sw::universal::fixpnt<1024, 32, Saturate, uint32_t> >, NR_OPS / 16);
}


// measure performance of arithmetic operations
void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nFIXPNT Fixed-Point Saturate Arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;
	PerformanceRunner("fixpnt<  8,  4, Saturate, uint8_t >  add/subtract    ", AdditionSubtractionWorkload< sw::universal::fixpnt<  8,  4, Saturate, uint8_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 16,  8, Saturate, uint16_t>  add/subtract    ", AdditionSubtractionWorkload< sw::universal::fixpnt< 16,  8, Saturate, uint16_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 32, 16, Saturate, uint32_t>  add/subtract    ", AdditionSubtractionWorkload< sw::universal::fixpnt< 32, 16, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 64, 32, Saturate, uint32_t>  add/subtract    ", AdditionSubtractionWorkload< sw::universal::fixpnt< 64, 32, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt<128, 32, Saturate, uint32_t>  add/subtract    ", AdditionSubtractionWorkload< sw::universal::fixpnt<128, 32, Saturate, uint32_t> >, NR_OPS / 2);

#if 0
	NR_OPS = 1024 * 32;
	PerformanceRunner("fixpnt<  8,  4, Saturate,uint8_t >  division        ", DivisionWorkload< sw::universal::fixpnt<  8,  4, Saturate, uint8_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 16,  8, Saturate,uint16_t>  division        ", DivisionWorkload< sw::universal::fixpnt< 16,  8, Saturate, uint16_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 32, 16, Saturate,uint32_t>  division        ", DivisionWorkload< sw::universal::fixpnt< 32, 16, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 64, 32, Saturate,uint32_t>  division        ", DivisionWorkload< sw::universal::fixpnt< 64, 32, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt<128, 32, Saturate,uint32_t>  division        ", DivisionWorkload< sw::universal::fixpnt<128, 32, Saturate, uint32_t> >, NR_OPS / 2);

	NR_OPS = 1024 * 32;
	PerformanceRunner("fixpnt<  8,  4, Saturate,uint8_t >  remainder       ", RemainderWorkload< sw::universal::fixpnt<  8,  4, Saturate, uint8_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 16,  8, Saturate,uint16_t>  remainder       ", RemainderWorkload< sw::universal::fixpnt< 16,  8, Saturate, uint16_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 32, 16, Saturate,uint32_t>  remainder       ", RemainderWorkload< sw::universal::fixpnt< 32, 16, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 64, 32, Saturate,uint32_t>  remainder       ", RemainderWorkload< sw::universal::fixpnt< 64, 32, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt<128, 32, Saturate,uint32_t>  remainder       ", RemainderWorkload< sw::universal::fixpnt<128, 32, Saturate, uint32_t> >, NR_OPS / 2);
#endif
	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("fixpnt<  8,  4, Saturate, uint8_t >  multiplication ", MultiplicationWorkload< sw::universal::fixpnt<  8,  4, Saturate, uint8_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 16,  8, Saturate, uint16_t>  multiplication ", MultiplicationWorkload< sw::universal::fixpnt< 16,  8, Saturate, uint16_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 32, 16  Saturate, uint32_t>  multiplication ", MultiplicationWorkload< sw::universal::fixpnt< 32, 16, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt< 64, 32, Saturate, uint32_t>  multiplication ", MultiplicationWorkload< sw::universal::fixpnt< 64, 32, Saturate, uint32_t> >, NR_OPS);
	PerformanceRunner("fixpnt<128, 32, Saturate, uint32_t>  multiplication ", MultiplicationWorkload< sw::universal::fixpnt<128, 32, Saturate, uint32_t> >, NR_OPS / 2);
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "Integer operator performance benchmarking";

#if MANUAL_TESTING

	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestShiftOperatorPerformance();
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
