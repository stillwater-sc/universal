//  performance.cpp : performance benchmarking for internal blocktriple operators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bitcast.hpp>
#include <iostream>
#include <string>
#include <chrono>

#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/blockbinary_test_status.hpp>
#include <universal/verification/performance_runner.hpp>

namespace sw::universal::internal {

	// Generic set of adds and subtracts for a given number system type
	template<typename Scalar>
	void AdditionSubtractionWorkload(uint64_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.clear();
		d.setbits(0xFFFF'FFFF'FFFF'FFFFull);
		c = d;
		b = c;
		a = b;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			c.add(a, b);
		}
		if (c == d) std::cout << "amazing\n";
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(uint64_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.clear();
		d.setbits(0xFFFF'FFFF'FFFF'FFFFull);
		c = d;
		b = c;
		a = b;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			c.mul(a, b);
			d = c;
		}
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(uint64_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.clear();
		d.setbits(0xFFFF'FFFF'FFFF'FFFFull);
		c = d;
		b = c;
		a = b;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			//c = a / b;
			c.clear(); // reset to zero so d = c is fast
			d = c;
		}
	}

	void TestSmallArithmeticOperatorPerformance() {
		using namespace sw::universal;
		std::cout << "\nArithmetic operator performance\n";

		size_t NR_OPS = 1024ull * 1024ull * 4ull;
		PerformanceRunner("blocktriple<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("blocktriple<16>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::MUL, uint32_t> >, NR_OPS / 2);

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

		NR_OPS = 1024ull * 512ull;
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
	*/
	void TestBlockPerformanceOnAdd() {
		using namespace sw::universal;
		std::cout << "\nADDITION: blocktriple arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 32ull * 1024ull * 1024ull;

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

		constexpr size_t NR_OPS = 1024ull * 1024;
		PerformanceRunner("blocktriple<4,uint8>      div   ", DivisionWorkload< sw::universal::blocktriple<4, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8>      div   ", DivisionWorkload< sw::universal::blocktriple<8, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024,uint8>   div   ", DivisionWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::DIV, uint8_t> >, NR_OPS / 16);
	}

	void TestBlockPerformanceOnMul() {
		using namespace sw::universal;
		std::cout << "\nMULTIPLICATION: blocktriple arithemetic performance as a function of size and BlockType\n";

		constexpr size_t NR_OPS = 512ull * 1024;
		PerformanceRunner("blocktriple<4,uint8>      mul   ", MultiplicationWorkload< sw::universal::blocktriple<4, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8>      mul   ", MultiplicationWorkload< sw::universal::blocktriple<8, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<16, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<32, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<64, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<128, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<256, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 16);
		PerformanceRunner("blocktriple<512,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<512, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 512);
		PerformanceRunner("blocktriple<1024,uint8>   mul   ", MultiplicationWorkload< sw::universal::blocktriple<1024, sw::universal::BlockTripleOperator::MUL, uint8_t> >, NR_OPS / 1024);

	}
}

// to generate a test report, set MANUAL_TESTING to 0 and STRESS_TESTING to 1
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal::internal;

	std::string tag = "blocktriple operator performance benchmarking";

#if MANUAL_TESTING

	TestSmallArithmeticOperatorPerformance();
	
	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;

	TestArithmeticOperatorPerformance();

#if STRESS_TESTING

	TestBlockPerformanceOnAdd();
	TestBlockPerformanceOnMul();
	TestBlockPerformanceOnDiv();

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
Date run : 3/01/2021
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/
