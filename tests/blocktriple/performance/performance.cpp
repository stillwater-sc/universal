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
		d.setbits(0xFFFFFFFFFFFFFFFFull);
		a = b = c = d;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			//sw::universal::module_add(a, b, c);
			//a = c - b;
		}
	}

	// Generic set of multiplies for a given number system type
	template<typename Scalar>
	void MultiplicationWorkload(uint64_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.setbits(0xFFFFFFFFFFFFFFFFull);
		a = b = c = d;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			//c = a * b;
			c.clear(); // reset to zero so d = c is fast
			d = c;
		}
	}

	// Generic set of divides for a given number system type
	template<typename Scalar>
	void DivisionWorkload(uint64_t NR_OPS) {
		Scalar a{ 0 }, b{ 0 }, c{ 0 }, d{ 0 };
		d.setbits(0xFFFFFFFFFFFFFFFFull);
		a = b = c = d;
		for (uint64_t i = 0; i < NR_OPS; ++i) {
			//c = a / b;
			c.clear(); // reset to zero so d = c is fast
			d = c;
		}
	}


	void TestArithmeticOperatorPerformance() {
		using namespace std;
		using namespace sw::universal;
		cout << endl << "Arithmetic operator performance" << endl;

		size_t NR_OPS = 1024ull * 1024ull * 4ull;

		PerformanceRunner("blocktriple<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<16> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<32> >, NR_OPS);
		PerformanceRunner("blocktriple<64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<64> >, NR_OPS);
		PerformanceRunner("blocktriple<128>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<128> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<256> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<512> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024> add/subtract  ", AdditionSubtractionWorkload< sw::universal::blocktriple<1024> >, NR_OPS / 16);

		NR_OPS = 1024ull * 1024ull;
		PerformanceRunner("blocktriple<16>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<16> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<32> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<64>   multiplication", MultiplicationWorkload< sw::universal::blocktriple<64> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<128>  multiplication", MultiplicationWorkload< sw::universal::blocktriple<128> >, NR_OPS / 64);
		PerformanceRunner("blocktriple<512>  multiplication", MultiplicationWorkload< sw::universal::blocktriple<512> >, NR_OPS / 512);     // TODO: why is this so slow?
		PerformanceRunner("blocktriple<1024> multiplication", MultiplicationWorkload< sw::universal::blocktriple<1024> >, NR_OPS / 1024);   // TODO: why is this so slow?

		NR_OPS = 1024ull * 512ull;
		PerformanceRunner("blocktriple<16>   division      ", DivisionWorkload< sw::universal::blocktriple<16> >, NR_OPS);
		PerformanceRunner("blocktriple<32>   division      ", DivisionWorkload< sw::universal::blocktriple<32> >, NR_OPS);
		PerformanceRunner("blocktriple<64>   division      ", DivisionWorkload< sw::universal::blocktriple<64> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<128>  division      ", DivisionWorkload< sw::universal::blocktriple<128> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512>  division      ", DivisionWorkload< sw::universal::blocktriple<512> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024> division      ", DivisionWorkload< sw::universal::blocktriple<1024> >, NR_OPS / 16);
	}

	void TestBlockPerformanceOnAdd() {
		using namespace std;
		using namespace sw::universal;
		cout << endl << "ADDITION: blocktriple arithemetic performance as a function of size and BlockType" << endl;

		constexpr size_t NR_OPS = 32ull * 1024ull * 1024ull;

		PerformanceRunner("blocktriple<4,uint8>      add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<4> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8>      add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<8> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<16> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<32> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8>     add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<64> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<128> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<256> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512,uint8>    add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<512> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024,uint8>   add   ", AdditionSubtractionWorkload< sw::universal::blocktriple<1024> >, NR_OPS / 16);
	}

	void TestBlockPerformanceOnDiv() {
		using namespace std;
		using namespace sw::universal;
		cout << endl << "DIVISION: blocktriple arithemetic performance as a function of size and BlockType" << endl;

		constexpr size_t NR_OPS = 1024ull * 1024;
		PerformanceRunner("blocktriple<4,uint8>      div   ", DivisionWorkload< sw::universal::blocktriple<4> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8>      div   ", DivisionWorkload< sw::universal::blocktriple<8> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<16> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<32> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8>     div   ", DivisionWorkload< sw::universal::blocktriple<64> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<128> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<256> >, NR_OPS / 4);
		PerformanceRunner("blocktriple<512,uint8>    div   ", DivisionWorkload< sw::universal::blocktriple<512> >, NR_OPS / 8);
		PerformanceRunner("blocktriple<1024,uint8>   div   ", DivisionWorkload< sw::universal::blocktriple<1024> >, NR_OPS / 16);
	}

	void TestBlockPerformanceOnMul() {
		using namespace std;
		using namespace sw::universal;
		cout << endl << "MULTIPLICATION: blocktriple arithemetic performance as a function of size and BlockType" << endl;

		constexpr size_t NR_OPS = 512ull * 1024;
		PerformanceRunner("blocktriple<4,uint8>      mul   ", MultiplicationWorkload< sw::universal::blocktriple<4> >, NR_OPS);
		PerformanceRunner("blocktriple<8,uint8>      mul   ", MultiplicationWorkload< sw::universal::blocktriple<8> >, NR_OPS);
		PerformanceRunner("blocktriple<16,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<16> >, NR_OPS);
		PerformanceRunner("blocktriple<32,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<32> >, NR_OPS);
		PerformanceRunner("blocktriple<64,uint8>     mul   ", MultiplicationWorkload< sw::universal::blocktriple<64> >, NR_OPS);
		PerformanceRunner("blocktriple<128,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<128> >, NR_OPS / 2);
		PerformanceRunner("blocktriple<256,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<256> >, NR_OPS / 16);
		PerformanceRunner("blocktriple<512,uint8>    mul   ", MultiplicationWorkload< sw::universal::blocktriple<512> >, NR_OPS / 512);
		PerformanceRunner("blocktriple<1024,uint8>   mul   ", MultiplicationWorkload< sw::universal::blocktriple<1024> >, NR_OPS / 1024);

	}
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal::internal;

	std::string tag = "Integer operator performance benchmarking";

#if MANUAL_TESTING

	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	ShiftPerformanceWorkload< sw::universal::blocktriple<8> >(1);
	
	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;

	TestArithmeticOperatorPerformance();

	TestBlockPerformanceOnAdd();
	TestBlockPerformanceOnMul();
	TestBlockPerformanceOnDiv();

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
Date run : 3/01/2021
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

*/
