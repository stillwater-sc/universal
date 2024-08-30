// performance.cpp : performance benchmarking for abitrary fixed-precision reals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <chrono>
// configure the areal arithmetic class
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/benchmark/performance_runner.hpp>

/*
   The goal of the arbitrary fixed-precision reals is to provide a constrained 
   linear floating-point type to explore the benefits of multi-precision algorithms.
*/

// measure performance of conversion operators
void TestConversionPerformance() {
	using namespace sw::universal;
	std::cout << "\nAREAL Conversion operator performance\n";

	//uint64_t NR_OPS = 1000000;
}

template<typename Scalar>
void DecodeWorkload(size_t NR_OPS) {
	using namespace sw::universal;

	Scalar a{ 0 };
	size_t success{ 0 };
	bool first{ true };
	for (size_t i = 0; i < NR_OPS; ++i) {
		a.setbits(i);
		bool s{ false };
		blockbinary<a.es, typename Scalar::BlockType> e;
		blockbinary<a.fbits, typename Scalar::BlockType> f;
		bool ubit{ false };
		sw::universal::decode(a, s, e, f, ubit);
		if ((i % 2) == ubit) {
			++success;
		}
		else { 
			// this shouldn't happen, but found a bug this way with areal<64,11,uint64_t> as type
			if (first) {
				first = false;
				std::cout << typeid(a).name() << " :\n"
					<< to_binary(a,true) << '\n'
					<< "sign    : " << (s ? "-1\n" : "+1\n") 
					<< "exponent: " << to_binary(e,true) << '\n'
					<< "fraction: " << to_binary(f,true) << '\n'
					<< "ubit    : " << (ubit ? "1" : "0") << '\n';
			}
		}
	}
	if (success == 0) std::cout << "DECODE FAIL\n"; // just a quick double check that all went well
}

// measure performance of conversion operators
void TestDecodePerformance() {
	using namespace sw::universal;
	std::cout << "\nAREAL decode operator performance\n";

	uint64_t NR_OPS = 1000000;
	PerformanceRunner("areal<8,2,uint8_t>      decode         ", DecodeWorkload< sw::universal::areal<8, 2, uint8_t> >, NR_OPS);
	PerformanceRunner("areal<16,5,uint16_t>    decode         ", DecodeWorkload< sw::universal::areal<16, 5, uint16_t> >, NR_OPS);
	PerformanceRunner("areal<32,8,uint32_t>    decode         ", DecodeWorkload< sw::universal::areal<32, 8, uint32_t> >, NR_OPS);
//	PerformanceRunner("areal<64,11,uint64_t>   decode         ", DecodeWorkload< sw::universal::areal<64, 11, uint64_t> >, NR_OPS); until we fix the shift constexpr guard
/* 
1/4/2021
AREAL decode operator performance: this is a decode that enumerates the bits, thus slowest possible algorithm
areal<8,2,uint8_t>      decode             1000000 per        0.012412sec ->  80 Mops/sec
areal<16,5,uint16_t>    decode             1000000 per       0.0287893sec ->  34 Mops/sec
areal<32,8,uint32_t>    decode             1000000 per       0.0649867sec ->  15 Mops/sec
areal<64,11,uint64_t>   decode             1000000 per        0.129481sec ->   7 Mops/sec

1/5/2021
AREAL decode operator performance: this is an exponent block move if there is no straddle
areal<8,2,uint8_t>      decode             1000000 per       0.0082558sec -> 121 Mops/sec
areal<16,5,uint16_t>    decode             1000000 per       0.0185946sec ->  53 Mops/sec
areal<32,8,uint32_t>    decode             1000000 per       0.0465827sec ->  21 Mops/sec
areal<64,11,uint64_t>   decode             1000000 per        0.104031sec ->   9 Mops/sec

2/26/2021
AREAL decode operator performance                                                               <--- this includes setbits() so we have more dynamic behavior of the test
areal<8,2,uint8_t>      decode             1000000 per       0.0017149sec -> 583 Mops/sec
areal<16,5,uint16_t>    decode             1000000 per       0.0015602sec -> 640 Mops/sec
areal<32,8,uint32_t>    decode             1000000 per       0.0021211sec -> 471 Mops/sec
areal<64,11,uint64_t>   decode             1000000 per       0.0017222sec -> 580 Mops/sec
*/
}

// measure performance of arithmetic operators
void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nAREAL Arithmetic operator performance\n";

	uint64_t NR_OPS = 1000000;

	PerformanceRunner("areal<8,2,uint8_t>      add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<8,2,uint8_t> >, NR_OPS);
	PerformanceRunner("areal<16,5,uint16_t>    add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<32,8,uint32_t>    add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<32,8,uint32_t> >, NR_OPS);
//	PerformanceRunner("areal<64,11,uint64_t>   add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<64,11,uint64_t> >, NR_OPS);
//	PerformanceRunner("areal<128,15,uint64_t>  add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("areal<256,15,uint64_t   add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("areal<512,15,uint64_t>  add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("areal<1024,15,uint64_t> add/subtract   ", AdditionSubtractionWorkload< sw::universal::areal<1024,15,uint64_t> >, NR_OPS / 16);

	NR_OPS = 1024 * 32;
	PerformanceRunner("areal<8,2,uint16_t>     division       ", DivisionWorkload< sw::universal::areal<8,2,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<16,5,uint16_t>    division       ", DivisionWorkload< sw::universal::areal<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<32,8,uint32_t>    division       ", DivisionWorkload< sw::universal::areal<32,8,uint32_t> >, NR_OPS);
//	PerformanceRunner("areal<64,11,uint64_t>   division       ", DivisionWorkload< sw::universal::areal<64,11,uint64_t> >, NR_OPS);
//	PerformanceRunner("areal<128,15,uint64_t>  division       ", DivisionWorkload< sw::universal::areal<128,15,uint64_t> >, NR_OPS / 2);
//	PerformanceRunner("areal<256,15,uint64_t   division       ", DivisionWorkload< sw::universal::areal<256,15,uint64_t> >, NR_OPS / 4);
//	PerformanceRunner("areal<512,15,uint64_t>  division       ", DivisionWorkload< sw::universal::areal<512,15,uint64_t> >, NR_OPS / 8);
//	PerformanceRunner("areal<1024,15,uint64_t> division       ", DivisionWorkload< sw::universal::areal<1024,15,uint64_t> >, NR_OPS / 16);

	// multiplication is the slowest operator

	NR_OPS = 1024 * 32;
	PerformanceRunner("areal<8,2,uint16_t>     multiplication ", MultiplicationWorkload< sw::universal::areal<8,2,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<16,5,uint16_t>    multiplication ", MultiplicationWorkload< sw::universal::areal<16,5,uint16_t> >, NR_OPS);
	PerformanceRunner("areal<32,8,uint32_t>    multiplication ", MultiplicationWorkload< sw::universal::areal<32,8,uint32_t> >, NR_OPS);
//	PerformanceRunner("areal<64,11,uint64_t>   multiplication ", MultiplicationWorkload< sw::universal::areal<64,11,uint64_t> >, NR_OPS);
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
	using namespace sw::universal;

	std::string tag = "AREAL operator performance benchmarking";

#if MANUAL_TESTING

	using Scalar = areal<64, 11, uint64_t>;
	Scalar a;
	a.setbits(0xEEEEEEEEEEEEEEEEull);
	bool s{ false };
	blockbinary<a.es, typename Scalar::BlockType> e;
	blockbinary<a.fbits, typename Scalar::BlockType> f;
	bool ubit{ false };
	sw::universal::decode(a, s, e, f, ubit);
	std::cout << typeid(a).name() << " :\n"
		<< to_binary(a, true) << '\n'
		<< "sign    : " << (s ? "-1\n" : "+1\n")
		<< "exponent: " << to_binary(e, true) << '\n'
		<< "fraction: " << to_binary(f, true) << '\n'
		<< "ubit    : " << (ubit ? "1" : "0") << '\n';

	std::cout << "nbits: " << a.nbits << '\n';
	std::cout << "es   : " << a.es << '\n';
	std::cout << "fbits: " << a.fbits << '\n';

	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestDecodePerformance();
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
