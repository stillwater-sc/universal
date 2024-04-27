// performance.cpp : baseline performance benchmarking of arithmetic operators on the lns arithmetic type
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

#include <universal/number/lns/lns.hpp>
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
		if (c == d) std::cerr << "amazing\n";
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
		if (c == d) std::cerr << "amazing\n";
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
		if (c == d) std::cerr << "amazing\n";
	}

	template<typename Scalar>
	void AssignmentCopyWorkload(size_t NR_OPS) {
		constexpr size_t VECTOR_SIZE = 1024;
		constexpr size_t BLOCK_SIZE = 16;
		std::vector<Scalar> a(VECTOR_SIZE + BLOCK_SIZE), b(VECTOR_SIZE + BLOCK_SIZE);
		for (size_t i = 0; i < VECTOR_SIZE; ++i) {
			a[i] = Scalar(float(i));
		}

		size_t nrIterations = NR_OPS / BLOCK_SIZE;
		for (size_t i = 0; i < nrIterations; ++i) {
			for (size_t j = 0; j < BLOCK_SIZE; ++j) {
				b[i % VECTOR_SIZE + j] = a[i % VECTOR_SIZE + j] * b[i % VECTOR_SIZE + j];
			}
		}
		if (b[VECTOR_SIZE / 2] == 0.1) std::cerr << "amazing\n";
	}

	/*
	July 5th, 2022, Dell i7 desktop

	Arithmetic operator performance
	lns< 4, 1, uint8_t >   add/subtract       1048576 per        0.192855sec ->   5 Mops/sec
	lns< 8, 3, uint8_t >   add/subtract       1048576 per        0.236651sec ->   4 Mops/sec
	lns<12, 4, uint8_t >   add/subtract       1048576 per        0.265168sec ->   3 Mops/sec
	lns<12, 4, uint16_t>   add/subtract       1048576 per        0.256703sec ->   4 Mops/sec
	lns<16, 5, uint16_t>   add/subtract       1048576 per        0.239356sec ->   4 Mops/sec
	lns<16, 5, uint32_t>   add/subtract       1048576 per        0.237923sec ->   4 Mops/sec
	lns<20, 6, uint32_t>   add/subtract       1048576 per        0.239781sec ->   4 Mops/sec
	lns<32, 8, uint32_t>   add/subtract       1048576 per        0.291539sec ->   3 Mops/sec

	lns< 4, 1, uint8_t >   multiplication     1048576 per       0.0015025sec -> 697 Mops/sec
	lns< 8, 3, uint8_t >   multiplication     1048576 per       0.0011845sec -> 885 Mops/sec
	lns<12, 4, uint8_t >   multiplication     1048576 per        0.002029sec -> 516 Mops/sec
	lns<12, 4, uint16_t>   multiplication     1048576 per        0.001177sec -> 890 Mops/sec
	lns<16, 5, uint16_t>   multiplication     1048576 per       0.0011724sec -> 894 Mops/sec
	lns<16, 5, uint32_t>   multiplication     1048576 per       0.0012224sec -> 857 Mops/sec
	lns<20, 6, uint32_t>   multiplication     1048576 per       0.0013361sec -> 784 Mops/sec
	lns<32, 8, uint32_t>   multiplication     1048576 per       0.0013939sec -> 752 Mops/sec

	lns< 4, 1, uint8_t >   division           1048576 per        0.001553sec -> 675 Mops/sec
	lns< 8, 3, uint8_t >   division           1048576 per       0.0019071sec -> 549 Mops/sec
	lns<12, 4, uint8_t >   division           1048576 per       0.0207542sec ->  50 Mops/sec
	lns<12, 4, uint16_t>   division           1048576 per       0.0022975sec -> 456 Mops/sec
	lns<16, 5, uint16_t>   division           1048576 per       0.0020986sec -> 499 Mops/sec
	lns<16, 5, uint32_t>   division           1048576 per       0.0058155sec -> 180 Mops/sec
	lns<20, 6, uint32_t>   division           1048576 per       0.0018611sec -> 563 Mops/sec
	lns<32, 8, uint32_t>   division           1048576 per       0.0016528sec -> 634 Mops/sec

	October 24th, NZXT AMD Ryzen 7 2700X Eight-Core Processor, 3.70 GHz 
	5-10x slower compared to i7 for some reason
	
	Arithmetic operator performance
	lns< 4, 1, uint8_t >   add/subtract       1048576 per         2.39943sec -> 437 Kops/sec
	lns< 8, 3, uint8_t >   add/subtract       1048576 per         4.06563sec -> 257 Kops/sec
	lns<12, 4, uint8_t >   add/subtract       1048576 per         4.12357sec -> 254 Kops/sec
	lns<12, 4, uint16_t>   add/subtract       1048576 per         4.08521sec -> 256 Kops/sec
	lns<16, 5, uint16_t>   add/subtract       1048576 per         4.09803sec -> 255 Kops/sec
	lns<16, 5, uint32_t>   add/subtract       1048576 per         4.11181sec -> 255 Kops/sec
	lns<20, 6, uint32_t>   add/subtract       1048576 per         4.29146sec -> 244 Kops/sec
	lns<32, 8, uint32_t>   add/subtract       1048576 per         4.33838sec -> 241 Kops/sec

	lns< 4, 1, uint8_t >   multiplication     1048576 per       0.0082187sec -> 127 Mops/sec
	lns< 8, 3, uint8_t >   multiplication     1048576 per       0.0040326sec -> 260 Mops/sec
	lns<12, 4, uint8_t >   multiplication     1048576 per       0.0085892sec -> 122 Mops/sec
	lns<12, 4, uint16_t>   multiplication     1048576 per       0.0031166sec -> 336 Mops/sec
	lns<16, 5, uint16_t>   multiplication     1048576 per       0.0035843sec -> 292 Mops/sec
	lns<16, 5, uint32_t>   multiplication     1048576 per       0.0027879sec -> 376 Mops/sec
	lns<20, 6, uint32_t>   multiplication     1048576 per       0.0027407sec -> 382 Mops/sec
	lns<32, 8, uint32_t>   multiplication     1048576 per       0.0112044sec ->  93 Mops/sec

	lns< 4, 1, uint8_t >   division           1048576 per       0.0088832sec -> 118 Mops/sec
	lns< 8, 3, uint8_t >   division           1048576 per       0.0276199sec ->  37 Mops/sec
	lns<12, 4, uint8_t >   division           1048576 per       0.0635278sec ->  16 Mops/sec
	lns<12, 4, uint16_t>   division           1048576 per       0.0101647sec -> 103 Mops/sec
	lns<16, 5, uint16_t>   division           1048576 per       0.0285868sec ->  36 Mops/sec
	lns<16, 5, uint32_t>   division           1048576 per       0.0579237sec ->  18 Mops/sec
	lns<20, 6, uint32_t>   division           1048576 per        0.008636sec -> 121 Mops/sec
	lns<32, 8, uint32_t>   division           1048576 per       0.0280106sec ->  37 Mops/sec
	*/

	void TestSmallArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 4 * 1024ull;
		PerformanceRunner("lns< 4, 1, uint8_t >   add/subtract   ", AdditionSubtractionWorkload< lns< 4, 1, uint8_t> >, NR_OPS);
		PerformanceRunner("lns< 8, 3, uint8_t >   add/subtract   ", AdditionSubtractionWorkload< lns< 8, 3, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<12, 4, uint8_t >   add/subtract   ", AdditionSubtractionWorkload< lns<12, 4, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<12, 4, uint16_t>   add/subtract   ", AdditionSubtractionWorkload< lns<12, 4, uint16_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint16_t>   add/subtract   ", AdditionSubtractionWorkload< lns<16, 5, uint16_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint32_t>   add/subtract   ", AdditionSubtractionWorkload< lns<16, 5, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<20, 6, uint32_t>   add/subtract   ", AdditionSubtractionWorkload< lns<20, 6, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<32, 8, uint32_t>   add/subtract   ", AdditionSubtractionWorkload< lns<32, 8, uint32_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("lns< 4, 1, uint8_t >   multiplication ", MultiplicationWorkload< lns< 4, 1, uint8_t> >, NR_OPS);
		PerformanceRunner("lns< 8, 3, uint8_t >   multiplication ", MultiplicationWorkload< lns< 8, 3, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<12, 4, uint8_t >   multiplication ", MultiplicationWorkload< lns<12, 4, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<12, 4, uint16_t>   multiplication ", MultiplicationWorkload< lns<12, 4, uint16_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint16_t>   multiplication ", MultiplicationWorkload< lns<16, 5, uint16_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint32_t>   multiplication ", MultiplicationWorkload< lns<16, 5, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<20, 6, uint32_t>   multiplication ", MultiplicationWorkload< lns<20, 6, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<32, 8, uint32_t>   multiplication ", MultiplicationWorkload< lns<32, 8, uint32_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("lns< 4, 1, uint8_t >   division       ", DivisionWorkload< lns< 4, 1, uint8_t> >, NR_OPS);
		PerformanceRunner("lns< 8, 3, uint8_t >   division       ", DivisionWorkload< lns< 8, 3, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<12, 4, uint8_t >   division       ", DivisionWorkload< lns<12, 4, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<12, 4, uint16_t>   division       ", DivisionWorkload< lns<12, 4, uint16_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint16_t>   division       ", DivisionWorkload< lns<16, 5, uint16_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint32_t>   division       ", DivisionWorkload< lns<16, 5, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<20, 6, uint32_t>   division       ", DivisionWorkload< lns<20, 6, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<32, 8, uint32_t>   division       ", DivisionWorkload< lns<32, 8, uint32_t> >, NR_OPS);

	}

	/*
	July 5th, 2022, Dell i7 desktop

	Arithmetic operator performance
	lns<  8, 2, uint8_t >  add/subtract      1048576 per        0.233537sec ->   4 Mops/sec
	lns< 16, 5, uint16_t>  add/subtract      1048576 per        0.243072sec ->   4 Mops/sec
	lns< 32, 8, uint32_t>  add/subtract      1048576 per         0.29188sec ->   3 Mops/sec
	lns< 64,11, uint32_t>  add/subtract      1048576 per         0.34077sec ->   3 Mops/sec
	lns<128,15, uint32_t>  add/subtract       524288 per        0.230368sec ->   2 Mops/sec
	lns<  8, 2, uint8_t >  multiplication    1048576 per       0.0012331sec -> 850 Mops/sec
	lns< 16, 5, uint16_t>  multiplication    1048576 per        0.001179sec -> 889 Mops/sec
	lns< 32, 8, uint32_t>  multiplication    1048576 per       0.0013948sec -> 751 Mops/sec
	lns< 64,11, uint32_t>  multiplication    1048576 per       0.0079976sec -> 131 Mops/sec
	lns<128,15, uint32_t>  multiplication    1048576 per        0.010877sec ->  96 Mops/sec
	lns<  8, 2, uint8_t >  division          1048576 per       0.0019065sec -> 550 Mops/sec
	lns< 16, 5, uint16_t>  division          1048576 per       0.0020712sec -> 506 Mops/sec
	lns< 32, 8, uint32_t>  division          1048576 per       0.0016435sec -> 638 Mops/sec
	lns< 64,11, uint32_t>  division          1048576 per       0.0205989sec ->  50 Mops/sec
	lns<128,15, uint32_t>  division          1048576 per       0.0252456sec ->  41 Mops/sec
	*/

	void TestArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 4 * 1024ull;
		PerformanceRunner("lns<  8, 2, uint8_t >  add/subtract  ", AdditionSubtractionWorkload< lns<  8,  2, uint8_t> >, NR_OPS);
		PerformanceRunner("lns< 16, 5, uint16_t>  add/subtract  ", AdditionSubtractionWorkload< lns< 16,  5, uint16_t> >, NR_OPS);
		PerformanceRunner("lns< 32, 8, uint32_t>  add/subtract  ", AdditionSubtractionWorkload< lns< 32,  8, uint32_t> >, NR_OPS);
		PerformanceRunner("lns< 64,11, uint32_t>  add/subtract  ", AdditionSubtractionWorkload< lns< 64, 11, uint32_t> >, NR_OPS);
		PerformanceRunner("lns< 96,32, uint32_t>  add/subtract  ", AdditionSubtractionWorkload< lns< 96, 32, uint32_t> >, NR_OPS);

		// an lns<128,rbits> is always going to be too big to represent exponents in 64bit native
		//PerformanceRunner("lns<128,15, uint32_t>  add/subtract  ", AdditionSubtractionWorkload< lns<128, 15, uint32_t> >, NR_OPS / 2);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("lns<  8, 2, uint8_t >  multiplication", MultiplicationWorkload< lns<  8,  2, uint8_t> >, NR_OPS);
		PerformanceRunner("lns< 16, 5, uint16_t>  multiplication", MultiplicationWorkload< lns< 16,  5, uint16_t> >, NR_OPS);
		PerformanceRunner("lns< 32, 8, uint32_t>  multiplication", MultiplicationWorkload< lns< 32,  8, uint32_t> >, NR_OPS);
		PerformanceRunner("lns< 64,11, uint32_t>  multiplication", MultiplicationWorkload< lns< 64, 11, uint32_t> >, NR_OPS);
		PerformanceRunner("lns< 96,32, uint32_t>  add/subtract  ", MultiplicationWorkload< lns< 96, 32, uint32_t> >, NR_OPS);
		// configuration too big
		//PerformanceRunner("lns<128,15, uint32_t>  multiplication", MultiplicationWorkload< lns<128, 15, uint32_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("lns<  8, 2, uint8_t >  division      ", DivisionWorkload< lns<  8,  2, uint8_t> >, NR_OPS);
		PerformanceRunner("lns< 16, 5, uint16_t>  division      ", DivisionWorkload< lns< 16,  5, uint16_t> >, NR_OPS);
		PerformanceRunner("lns< 32, 8, uint32_t>  division      ", DivisionWorkload< lns< 32,  8, uint32_t> >, NR_OPS);
		PerformanceRunner("lns< 64,11, uint32_t>  division      ", DivisionWorkload< lns< 64, 11, uint32_t> >, NR_OPS);
		PerformanceRunner("lns< 96,32, uint32_t>  add/subtract  ", DivisionWorkload< lns< 96, 32, uint32_t> >, NR_OPS);
		// configuration too big
		//PerformanceRunner("lns<128,15, uint32_t>  division      ", DivisionWorkload< lns<128, 15, uint32_t> >, NR_OPS);
	}

	/*
	July 5th, 2022, Dell i7 desktop

	ASSIGNMENT/COPY: lns assignment performance as a function of size and BlockType
	lns< 8, 2, uint8_t>    assignment/copy      33554432 per       0.0315538sec ->   1 Gops/sec
	lns<16, 5, uint8_t>    assignment/copy      33554432 per        0.157411sec -> 213 Mops/sec
	lns<32, 8, uint8_t>    assignment/copy      33554432 per        0.315398sec -> 106 Mops/sec
	lns< 8, 2, uint32_t>   assignment/copy      33554432 per       0.0316567sec ->   1 Gops/sec
	lns<16, 5, uint32_t>   assignment/copy      33554432 per       0.0858747sec -> 390 Mops/sec
	lns<32, 8, uint32_t>   assignment/copy      33554432 per       0.0538612sec -> 622 Mops/sec
	lns< 8, 2, uint64_t>   assignment/copy      33554432 per       0.0326031sec ->   1 Gops/sec
	lns<16, 5, uint64_t>   assignment/copy      33554432 per       0.0525943sec -> 637 Mops/sec
	lns<32, 8, uint64_t>   assignment/copy      33554432 per        0.157412sec -> 213 Mops/sec
	*/
	void TestAssignmentCopyPerformanceOn() {
		using namespace sw::universal;
		std::cout << "\nASSIGNMENT/COPY: lns assignment performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 32ull * 1024ull * 1024ull;

		PerformanceRunner("lns< 8, 2, uint8_t>    assignment/copy   ", AssignmentCopyWorkload< lns< 8, 2, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint8_t>    assignment/copy   ", AssignmentCopyWorkload< lns<16, 5, uint8_t> >, NR_OPS);
		PerformanceRunner("lns<32, 8, uint8_t>    assignment/copy   ", AssignmentCopyWorkload< lns<32, 8, uint8_t> >, NR_OPS);

		PerformanceRunner("lns< 8, 2, uint32_t>   assignment/copy   ", AssignmentCopyWorkload< lns< 8, 2, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint32_t>   assignment/copy   ", AssignmentCopyWorkload< lns<16, 5, uint32_t> >, NR_OPS);
		PerformanceRunner("lns<32, 8, uint32_t>   assignment/copy   ", AssignmentCopyWorkload< lns<32, 8, uint32_t> >, NR_OPS);

		PerformanceRunner("lns< 8, 2, uint64_t>   assignment/copy   ", AssignmentCopyWorkload< lns< 8, 2, uint64_t> >, NR_OPS);
		PerformanceRunner("lns<16, 5, uint64_t>   assignment/copy   ", AssignmentCopyWorkload< lns<16, 5, uint64_t> >, NR_OPS);
		PerformanceRunner("lns<32, 8, uint64_t>   assignment/copy   ", AssignmentCopyWorkload< lns<32, 8, uint64_t> >, NR_OPS);
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

	std::string test_suite  = "lns operator performance benchmarking";
	std::string test_tag    = "performance";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	TestSmallArithmeticOperatorPerformance();
	
	TestArithmeticOperatorPerformance();

	TestAssignmentCopyPerformanceOn();

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

	TestAssignmentCopyPerformanceOn();
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
