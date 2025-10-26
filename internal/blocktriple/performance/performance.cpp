//  performance.cpp : performance benchmarking for internal blocktriple operators
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

#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/blockbinary_test_status.hpp>
#include <universal/benchmark/performance_runner.hpp>

namespace sw::universal::internal {

	// Generic set of adds and subtracts for a given number system type
	template<typename Scalar>
	void AdditionSubtractionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.clear();
		d.setbits(0xFFFF'FFFF'FFFF'FFFFull);
		c = d;
		b = c;
		a = b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c.add(a, b);
			b = c;
		}
		if (c == d) std::cout << "amazing\n";
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.clear();
		d.setbits(0xFFFF'FFFF'FFFF'FFFFull);
		c = d;
		b = c;
		a = b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c.mul(a, b);
			b = c;
		}
		if (c == d) std::cout << "amazing\n";
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(size_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.clear();
		d.setbits(0xFFFF'FFFF'FFFF'FFFFull);
		c = d;
		b = c;
		a = b;
		for (size_t i = 0; i < NR_OPS; ++i) {
			c.div(a, b);
			b = c;
		}
		if (c == d) std::cout << "amazing\n";
	}

	void TestSmallArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("blocktriple<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("blocktriple<16>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 2);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("blocktriple<16>   division      ", DivisionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   division      ", DivisionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::DIV, uint32_t> >, NR_OPS / 2);

	}

	void TestArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("blocktriple<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::MUL, uint64_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024> add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 16);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("blocktriple<16>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<64>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::MUL, uint64_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<128>  multiplication", MultiplicationWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 64);
		PerformanceRunner("blocktriple<512>  multiplication", MultiplicationWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 512);     // TODO: why is this so slow?
		PerformanceRunner("blocktriple<1024> multiplication", MultiplicationWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 1024);   // TODO: why is this so slow?

		NR_OPS = 1024ull * 32ull;
		PerformanceRunner("blocktriple<16>   division      ", DivisionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   division      ", DivisionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::DIV, uint32_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64>   division      ", DivisionWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::DIV, uint64_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<128>  division      ", DivisionWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::DIV, uint32_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512>  division      ", DivisionWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::DIV, uint32_t> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024> division      ", DivisionWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::DIV, uint32_t> >, NR_OPS / 16);
	}

	/*
	August 11, 2021: laptop:
	ADDITION: blocktriple arithemetic performance as a function of size and BlockType
	blocktriple<4,uint8_t>      add      33554432 per        0.271299sec -> 123 Mops/sec
	blocktriple<8,uint8_t>      add      33554432 per        0.369784sec ->  90 Mops/sec
	blocktriple<16,uint8_t>     add      33554432 per         0.49805sec ->  67 Mops/sec
	blocktriple<32,uint8_t>     add      33554432 per        0.926172sec ->  36 Mops/sec
	blocktriple<64,uint8_t>     add      33554432 per        0.272839sec -> 122 Mops/sec
	blocktriple<128,uint8_t>    add      16777216 per        0.574327sec ->  29 Mops/sec
	blocktriple<256,uint8_t>    add       8388608 per         1.51128sec ->   5 Mops/sec
	blocktriple<512,uint8_t>    add       4194304 per         1.47066sec ->   2 Mops/sec
	blocktriple<1024,uint8_t>   add       2097152 per        0.635263sec ->   3 Mops/sec

	blocktriple<4,uint32_t>     add      33554432 per        0.885621sec ->  37 Mops/sec
	blocktriple<8,uint32_t>     add      33554432 per        0.597587sec ->  56 Mops/sec
	blocktriple<16,uint32_t>    add      33554432 per        0.516732sec ->  64 Mops/sec
	blocktriple<32,uint32_t>    add      33554432 per        0.664498sec ->  50 Mops/sec
	blocktriple<64,uint32_t>    add      33554432 per         0.12037sec -> 278 Mops/sec
	blocktriple<128,uint32_t>   add      16777216 per        0.347922sec ->  48 Mops/sec
	blocktriple<256,uint32_t>   add       8388608 per        0.397124sec ->  21 Mops/sec
	blocktriple<512,uint32_t>   add       4194304 per        0.425287sec ->   9 Mops/sec
	blocktriple<1024,uint32_t>  add       2097152 per        0.157525sec ->  13 Mops/sec

	blocktriple<4,uint64_t>     add      33554432 per         1.25851sec ->  26 Mops/sec
	blocktriple<8,uint64_t>     add      33554432 per        0.889904sec ->  37 Mops/sec
	blocktriple<16,uint64_t>    add      33554432 per        0.879058sec ->  38 Mops/sec
	blocktriple<32,uint64_t>    add      33554432 per          1.0772sec ->  31 Mops/sec
	blocktriple<64,uint64_t>    add      33554432 per        0.369528sec ->  90 Mops/sec
	blocktriple<128,uint64_t>   add      16777216 per        0.187797sec ->  89 Mops/sec
	blocktriple<256,uint64_t>   add       8388608 per        0.097388sec ->  86 Mops/sec
	blocktriple<512,uint64_t>   add       4194304 per        0.101417sec ->  41 Mops/sec
	blocktriple<1024,uint64_t>  add       2097152 per       0.0758496sec ->  27 Mops/sec

	January, 2022, Dell i7 desktop
	blocktriple operator performance benchmarking
	blocktriple<16>   add/subtract      4194304 per        0.183834sec ->  22 Mops/sec
	blocktriple<32>   add/subtract      4194304 per        0.215027sec ->  19 Mops/sec
	blocktriple<64>   add/subtract      4194304 per       0.0425988sec ->  98 Mops/sec
	blocktriple<128>  add/subtract      2097152 per        0.387862sec ->   5 Mops/sec
	blocktriple<256>  add/subtract      1048576 per        0.434191sec ->   2 Mops/sec
	blocktriple<512>  add/subtract       524288 per        0.446962sec ->   1 Mops/sec
	blocktriple<1024> add/subtract       262144 per        0.475721sec -> 551 Kops/sec
	blocktriple<16>   multiplication    1048576 per        0.152838sec ->   6 Mops/sec
	blocktriple<32>   multiplication     524288 per       0.0828581sec ->   6 Mops/sec
	blocktriple<64>   multiplication     262144 per       0.0363935sec ->   7 Mops/sec
	blocktriple<128>  multiplication      16384 per       0.0164197sec -> 997 Kops/sec
	blocktriple<512>  multiplication       2048 per       0.0588853sec ->  34 Kops/sec
	blocktriple<1024> multiplication       1024 per        0.109377sec ->   9 Kops/sec
	blocktriple<16>   division           524288 per        0.389545sec ->   1 Mops/sec
	blocktriple<32>   division           524288 per        0.506002sec ->   1 Mops/sec
	blocktriple<64>   division           262144 per        0.285299sec -> 918 Kops/sec
	blocktriple<128>  division           131072 per           1.466sec ->  89 Kops/sec
	blocktriple<512>  division            65536 per         16.9605sec ->   3 Kops/sec
	blocktriple<1024> division            32768 per         33.7386sec -> 971  ops/sec
	*/
	void TestBlockPerformanceOnAdd() {
		using namespace sw::universal;
		std::cout << "\nADDITION: blocktriple arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 2ull * 1024ull * 1024ull;

		PerformanceRunner("blocktriple<4,uint8_t>      add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<4, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8_t>      add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<8, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8_t>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8_t>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8_t>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512,uint8_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024,uint8_t>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::ADD, uint8_t> >, NR_OPS / 16);

		PerformanceRunner("blocktriple<4,uint32_t>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<4, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint32_t>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<8, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint32_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint32_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint32_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512,uint32_t>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024,uint32_t>  add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::ADD, uint32_t> >, NR_OPS / 16);

		PerformanceRunner("blocktriple<4,uint64_t>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<4, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint64_t>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<8, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint64_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint64_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint64_t>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512,uint64_t>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024,uint64_t>  add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::ADD, uint64_t> >, NR_OPS / 16);
	}

	void TestBlockPerformanceOnDiv() {
		using namespace sw::universal;
		std::cout << "\nDIVISION: blocktriple arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 1024ull * 4;
		PerformanceRunner("blocktriple<4,uint8>      div   ", DivisionWorkload< sw::universal::blocktriple<4, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8>      div   ", DivisionWorkload< sw::universal::blocktriple<8, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 16);
		PerformanceRunner("blocktriple<1024,uint8>   div   ", DivisionWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 256);
	}

	void TestBlockPerformanceOnMul() {
		using namespace sw::universal;
		std::cout << "\nMULTIPLICATION: blocktriple arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 1024ull * 4;
		PerformanceRunner("blocktriple<4,uint8>      mul   ", MultiplicationWorkload< sw::universal::blocktriple<4, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8>      mul   ", MultiplicationWorkload< sw::universal::blocktriple<8, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 16);
		PerformanceRunner("blocktriple<512,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 16);
		PerformanceRunner("blocktriple<1024,uint8>   mul   ", MultiplicationWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 256);

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

	std::string test_suite = "blocktriple operator performance benchmarking";
//	std::string test_tag = "blocktriple performance";
//	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	TestSmallArithmeticOperatorPerformance();
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	TestSmallArithmeticOperatorPerformance();
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

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
