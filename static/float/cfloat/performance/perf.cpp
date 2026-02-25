//  perf.cpp : baseline performance benchmarking for cfloat arithmetic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <string>
#include <chrono>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/benchmark/performance_runner.hpp>

namespace sw::universal::internal {

	// Generic set of adds and subtracts for a given number system type
	template<typename Scalar>
	void AdditionSubtractionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1.0f;
		c = d;
		b = c;
		a = b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a + b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << "amazing\n";
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1.0f;
		c = d;
		b = 1.125f;
		a = 1.0f / b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a * b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << "amazing\n";
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d = 1.0f;
		c = d;
		b = 1.5f;
		a = 0.75f;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c = a / b;
			b = c;
		}
//		std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		if (c == d) std::cout << "amazing\n";
	}

	/*
	January, 2022, Dell i7 desktop
	Arithmetic operator performance
	cfloat<16, 5>   add/subtract      4194304 per        0.185824sec ->  22 Mops/sec
	cfloat<32, 8>   add/subtract      4194304 per        0.217755sec ->  19 Mops/sec
	cfloat<16, 5>   multiplication    1048576 per       0.0087288sec -> 120 Mops/sec
	cfloat<32, 8>   multiplication     524288 per       0.0032179sec -> 162 Mops/sec
	cfloat<16, 5>   division          1048576 per        0.416226sec ->   2 Mops/sec
	cfloat<32, 8>   division           524288 per        0.299649sec ->   1 Mops/sec
	 */

	void TestSmallArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("cfloat<16, 5>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat<32, 8>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::cfloat<32, 8, uint32_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("cfloat<16, 5>   multiplication", MultiplicationWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat<32, 8>   multiplication", MultiplicationWorkload< sw::universal::cfloat<32, 8, uint32_t> >, NR_OPS / 2);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("cfloat<16, 5>   division      ", DivisionWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat<32, 8>   division      ", DivisionWorkload< sw::universal::cfloat<32, 8, uint32_t> >, NR_OPS / 2);

	}

	/*
	January, 2022, Dell i7 desktop
	Arithmetic operator performance
	cfloat<16>   add/subtract      4194304 per        0.186801sec ->  22 Mops/sec
	cfloat<32>   add/subtract      4194304 per        0.219088sec ->  19 Mops/sec
	cfloat<64>   add/subtract      4194304 per         0.22908sec ->  18 Mops/sec
	cfloat<128>  add/subtract      2097152 per       0.0857134sec ->  24 Mops/sec
	cfloat<16>   multiplication    1048576 per       0.0087153sec -> 120 Mops/sec
	cfloat<32>   multiplication     524288 per       0.0031026sec -> 168 Mops/sec
	cfloat<64>   multiplication     262144 per       0.0026676sec ->  98 Mops/sec
	cfloat<128>  multiplication      16384 per       0.0151706sec ->   1 Mops/sec
	cfloat<16>   division           524288 per        0.208273sec ->   2 Mops/sec
	cfloat<32>   division           524288 per        0.301958sec ->   1 Mops/sec
	cfloat<64>   division           262144 per        0.516456sec -> 507 Kops/sec
	cfloat<128>  division           131072 per          1.1685sec -> 112 Kops/sec
	*/

	void TestArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("cfloat< 16, 5>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::cfloat<32, 8, uint32_t> >, NR_OPS);
		PerformanceRunner("cfloat< 64,11>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::cfloat<64, 11, uint32_t> >, NR_OPS);
		PerformanceRunner("cfloat<128,15>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::cfloat<128, 15, uint32_t> >, NR_OPS / 2);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("cfloat< 16, 5>   multiplication", MultiplicationWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8>   multiplication", MultiplicationWorkload< sw::universal::cfloat<32, 8, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("cfloat< 64,11>   multiplication", MultiplicationWorkload< sw::universal::cfloat<64, 11, uint32_t> >, NR_OPS / 4);
		PerformanceRunner("cfloat<128,15>   multiplication", MultiplicationWorkload< sw::universal::cfloat<128, 15, uint32_t> >, NR_OPS / 64);

		NR_OPS = 1024ull * 512ull;
		PerformanceRunner("cfloat< 16, 5>   division      ", DivisionWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8>   division      ", DivisionWorkload< sw::universal::cfloat<32, 8, uint32_t> >, NR_OPS);
		PerformanceRunner("cfloat< 64,11>   division      ", DivisionWorkload< sw::universal::cfloat<64, 11, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("cfloat<128,15>   division      ", DivisionWorkload< sw::universal::cfloat<128, 15, uint32_t> >, NR_OPS / 4);
	}

	/*
	January, 2022, Dell i7 desktop

	*/
	void TestBlockPerformanceOnAdd() {
		using namespace sw::universal;
		std::cout << "\nADDITION: cfloat arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 32ull * 1024ull * 1024ull;

		PerformanceRunner("cfloat<  8, 2, uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::cfloat<8, 2, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 16, 5, uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8, uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::cfloat<32, 8, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 64,11, uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::cfloat<64, 11, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat<128,15, uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::cfloat<128, 15, uint8_t> >, NR_OPS / 2);

		PerformanceRunner("cfloat<  8, 2, uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<8, 2, uint32_t> >, NR_OPS);
		PerformanceRunner("cfloat< 16, 5, uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<16, 5, uint32_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8, uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<32, 8, uint32_t> >, NR_OPS);
		PerformanceRunner("cfloat< 64,11, uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<64, 11, uint32_t> >, NR_OPS);
		PerformanceRunner("cfloat<128,15, uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<128, 15, uint32_t> >, NR_OPS / 2);

		PerformanceRunner("cfloat<  8, 2, uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<8, 2, uint64_t> >, NR_OPS);
		PerformanceRunner("cfloat< 16, 5, uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<16, 5, uint64_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8, uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<32, 8, uint64_t> >, NR_OPS);
		PerformanceRunner("cfloat< 64,11, uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<64, 11, uint64_t> >, NR_OPS);
		// this does not work!!! just a sense of performance. we don't have a mechanism to receive a carry from uint64_t limb arithmetic
		PerformanceRunner("cfloat<128,15, uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::cfloat<128, 15, uint64_t> >, NR_OPS / 2);
	}

	void TestBlockPerformanceOnDiv() {
		using namespace sw::universal;
		std::cout << "\nDIVISION: cfloat arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 1024ull * 1024;
		PerformanceRunner("cfloat<  8, 2, uint8_t>    div   ", DivisionWorkload< sw::universal::cfloat<8, 2, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 16, 5, uint8_t>    div   ", DivisionWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8, uint8_t>    div   ", DivisionWorkload< sw::universal::cfloat<32, 8, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 64,11, uint8_t>    div   ", DivisionWorkload< sw::universal::cfloat<64, 11, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat<128,15, uint8_t>    div   ", DivisionWorkload< sw::universal::cfloat<128, 15, uint8_t> >, NR_OPS / 2);
	}

	void TestBlockPerformanceOnMul() {
		using namespace sw::universal;
		std::cout << "\nMULTIPLICATION: cfloat arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 512ull * 1024;
		PerformanceRunner("cfloat<  8, 2, uint8_t>    mul   ", MultiplicationWorkload< sw::universal::cfloat<8, 2, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 16, 5, uint8_t>    mul   ", MultiplicationWorkload< sw::universal::cfloat<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 32, 8, uint8_t>    mul   ", MultiplicationWorkload< sw::universal::cfloat<32, 8, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat< 64,11, uint8_t>    mul   ", MultiplicationWorkload< sw::universal::cfloat<64, 11, uint8_t> >, NR_OPS);
		PerformanceRunner("cfloat<128,15, uint8_t>    mul   ", MultiplicationWorkload< sw::universal::cfloat<128, 15, uint8_t> >, NR_OPS / 2);
	}
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::internal;

	std::string test_suite  = "cfloat operator performance benchmarking";
	std::string test_tag    = "performance";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	TestSmallArithmeticOperatorPerformance();
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	TestSmallArithmeticOperatorPerformance();
#endif

#if REGRESSION_LEVEL_2
	TestArithmeticOperatorPerformance();
#endif

#if REGRESSION_LEVEL_4
	TestArithmeticOperatorPerformance();

	TestBlockPerformanceOnAdd();
	TestBlockPerformanceOnMul();
	TestBlockPerformanceOnDiv();
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
Date run : 3/01/2021
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/
