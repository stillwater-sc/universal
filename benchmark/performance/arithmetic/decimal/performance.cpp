// performance.cpp : performance benchmarking for adaptive precision decimal integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <chrono>
// configure the decimal arithmetic class
#define EDECIMAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

/*
   The goal of the adaptive decimal integers is to provide an oracle big integer type
*/

// test performance of shift operator on integer<> class
void TestShiftOperatorPerformance() {
	using namespace sw::universal;
	using Decimal = sw::universal::edecimal;
	std::cout << "edecimal logical shift operator performance\n";

	constexpr uint64_t NR_OPS = 1000;

	PerformanceRunner("edecimal 1-digits     shifts        ", ShiftPerformanceWorkload< Decimal >, NR_OPS);
	PerformanceRunner("edecimal 10-digits    shifts        ", ShiftPerformanceWorkload< Decimal >, NR_OPS);
	PerformanceRunner("edecimal 100-digits   shifts        ", ShiftPerformanceWorkload< Decimal >, NR_OPS);
}

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	using Decimal = sw::universal::edecimal;
	std::cout << "edecimal arithmetic operator performance\n";

	uint64_t NR_OPS = 1000;

	PerformanceRunner("edecimal 1-digits    add/subtract   ", AdditionSubtractionWorkload< Decimal >, NR_OPS);
	PerformanceRunner("edecimal 10-digits   add/subtract   ", AdditionSubtractionWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 100-digits  add/subtract   ", AdditionSubtractionWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 1000-digits add/subtract   ", AdditionSubtractionWorkload< Decimal >, NR_OPS);

	NR_OPS = 1024;
	PerformanceRunner("edecimal 1-digit     division       ", DivisionWorkload< Decimal >, NR_OPS);
	PerformanceRunner("edecimal 10-digit    division       ", DivisionWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 100-digit   division       ", DivisionWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 1000-digit  division       ", DivisionWorkload< Decimal >, NR_OPS);

	NR_OPS = 1024;
	PerformanceRunner("edecimal 11-digit    remainder      ", RemainderWorkload< Decimal >, NR_OPS);
	PerformanceRunner("edecimal 10-digit    remainder      ", RemainderWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 100-digit   remainder      ", RemainderWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 1000-digit  remainder      ", RemainderWorkload< Decimal >, NR_OPS);

	// multiplication is the slowest operator

	NR_OPS = 1024;
	PerformanceRunner("edecimal 1-digit     multiplication ", MultiplicationWorkload< Decimal >, NR_OPS);
	PerformanceRunner("edecimal 10-digit    multiplication ", MultiplicationWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 100-digit   multiplication ", MultiplicationWorkload< Decimal >, NR_OPS);
//	PerformanceRunner("edecimal 1000-digit  multiplication ", MultiplicationWorkload< Decimal >, NR_OPS);
}

// conditional compilation
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "edecimal big-integer operator performance benchmarking";

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

Integer operator performance benchmarking

Logical shift operator performance
integer<16>   shifts            1000000 per       0.0099091sec -> 100 Mops/sec
integer<32>   shifts            1000000 per       0.0453919sec ->  22 Mops/sec
integer<64>   shifts            1000000 per        0.178824sec ->   5 Mops/sec
integer<128>  shifts             500000 per        0.269217sec ->   1 Mops/sec
integer<256>  shifts             250000 per        0.266083sec -> 939 Kops/sec
integer<512>  shifts             125000 per        0.285764sec -> 437 Kops/sec
integer<1024> shifts              62500 per        0.277351sec -> 225 Kops/sec

Arithmetic operator performance
integer<16>   add/subtract      1000000 per       0.0095743sec -> 104 Mops/sec
integer<32>   add/subtract      1000000 per       0.0148453sec ->  67 Mops/sec
integer<64>   add/subtract      1000000 per       0.0474723sec ->  21 Mops/sec
integer<128>  add/subtract       500000 per       0.0382776sec ->  13 Mops/sec
integer<256>  add/subtract       250000 per       0.0268772sec ->   9 Mops/sec
integer<512>  add/subtract       125000 per       0.0275169sec ->   4 Mops/sec
integer<1024> add/subtract        62500 per       0.0294444sec ->   2 Mops/sec
integer<16>   division            32768 per       0.0050946sec ->   6 Mops/sec
integer<32>   division            32768 per       0.0083216sec ->   3 Mops/sec
integer<64>   division            16384 per       0.0074452sec ->   2 Mops/sec
integer<128>  division             8192 per       0.0071203sec ->   1 Mops/sec
integer<512>  division             4096 per       0.0148553sec -> 275 Kops/sec
integer<1024> division             2048 per       0.0154237sec -> 132 Kops/sec
integer<16>   remainder           32768 per       0.0051223sec ->   6 Mops/sec
integer<32>   remainder           32768 per       0.0082141sec ->   3 Mops/sec
integer<64>   remainder           16384 per       0.0077429sec ->   2 Mops/sec
integer<128>  remainder            8192 per       0.0078737sec ->   1 Mops/sec
integer<512>  remainder            4096 per       0.0148961sec -> 274 Kops/sec
integer<1024> remainder            2048 per       0.0150371sec -> 136 Kops/sec
integer<16>   multiplication      32768 per       0.0232329sec ->   1 Mops/sec
integer<32>   multiplication      16384 per       0.0424617sec -> 385 Kops/sec
integer<64>   multiplication       8192 per         0.08589sec ->  95 Kops/sec
integer<128>  multiplication       4096 per        0.166093sec ->  24 Kops/sec
integer<512>  multiplication       2048 per         1.33028sec ->   1 Kops/sec
integer<1024> multiplication       1024 per         2.58557sec -> 396  ops/sec
*/
