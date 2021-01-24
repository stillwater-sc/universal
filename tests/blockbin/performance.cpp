//  performance.cpp : performance benchmarking for block binary number arithmetic
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <chrono>

//#include <universal/integer/integer.hpp>
#include <universal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/blockbinary_test_status.hpp>
#include <universal/verification/performance_runner.hpp>





// test performance of shift operator on integer<> class
void TestShiftOperatorPerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "Logical shift operator performance" << endl;

	constexpr size_t NR_OPS = 1024ull * 1024ull;

	PerformanceRunner("blockbinary<16>   shifts        ", ShiftPerformanceWorkload< sw::universal::blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   shifts        ", ShiftPerformanceWorkload< sw::universal::blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   shifts        ", ShiftPerformanceWorkload< sw::universal::blockbinary<64> >, NR_OPS);
	PerformanceRunner("blockbinary<128>  shifts        ", ShiftPerformanceWorkload< sw::universal::blockbinary<128> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256>  shifts        ", ShiftPerformanceWorkload< sw::universal::blockbinary<256> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  shifts        ", ShiftPerformanceWorkload< sw::universal::blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> shifts        ", ShiftPerformanceWorkload< sw::universal::blockbinary<1024> >, NR_OPS / 16);
}

void TestBlockPerformanceOnShift() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "Block size performance on logical shift operators" << endl;

	constexpr size_t NR_OPS = 1024ull * 1024ull;

	PerformanceRunner("blockbinary<8,uint8>     shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<8, uint8_t> >, NR_OPS);

	PerformanceRunner("blockbinary<16,uint8>    shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<16, uint16_t> >, NR_OPS);

	PerformanceRunner("blockbinary<32,uint8>    shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<32, uint32_t> >, NR_OPS);

	PerformanceRunner("blockbinary<64,uint8>    shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<64, uint32_t> >, NR_OPS);

	PerformanceRunner("blockbinary<128,uint8>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<128, uint32_t> >, NR_OPS / 2);

	PerformanceRunner("blockbinary<256,uint8>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<256, uint32_t> >, NR_OPS / 4);

	PerformanceRunner("blockbinary<512,uint8>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<1024, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<1024, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<1024, uint32_t> >, NR_OPS / 8);

	PerformanceRunner("blockbinary<1024,uint8>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16> shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<1024, uint16_t> >, NR_OPS /16);
	PerformanceRunner("blockbinary<1024,uint32> shifts  ", ShiftPerformanceWorkload< sw::universal::blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestArithmeticOperatorPerformance() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "Arithmetic operator performance" << endl;

	size_t NR_OPS = 1024ull * 1024ull * 4ull;

	PerformanceRunner("blockbinary<16>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   add/subtract  ", AdditionSubtractionWorkload< sw::universal::blockbinary<64> >, NR_OPS);
	PerformanceRunner("blockbinary<128>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blockbinary<128> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blockbinary<256> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  add/subtract  ", AdditionSubtractionWorkload< sw::universal::blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> add/subtract  ", AdditionSubtractionWorkload< sw::universal::blockbinary<1024> >, NR_OPS / 16);

	NR_OPS = 1024ull * 1024ull;
	PerformanceRunner("blockbinary<16>   multiplication", MultiplicationWorkload< sw::universal::blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   multiplication", MultiplicationWorkload< sw::universal::blockbinary<32> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<64>   multiplication", MultiplicationWorkload< sw::universal::blockbinary<64> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<128>  multiplication", MultiplicationWorkload< sw::universal::blockbinary<128> >, NR_OPS / 64);
	PerformanceRunner("blockbinary<512>  multiplication", MultiplicationWorkload< sw::universal::blockbinary<512> >, NR_OPS / 512);     // TODO: why is this so slow?
	PerformanceRunner("blockbinary<1024> multiplication", MultiplicationWorkload< sw::universal::blockbinary<1024> >, NR_OPS / 1024);   // TODO: why is this so slow?

	NR_OPS = 1024ull * 512ull;
	PerformanceRunner("blockbinary<16>   division      ", DivisionWorkload< sw::universal::blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   division      ", DivisionWorkload< sw::universal::blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   division      ", DivisionWorkload< sw::universal::blockbinary<64> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128>  division      ", DivisionWorkload< sw::universal::blockbinary<128> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  division      ", DivisionWorkload< sw::universal::blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> division      ", DivisionWorkload< sw::universal::blockbinary<1024> >, NR_OPS / 16);

	NR_OPS = 1024ull * 512ull;
	PerformanceRunner("blockbinary<16>   remainder     ", RemainderWorkload< sw::universal::blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   remainder     ", RemainderWorkload< sw::universal::blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   remainder     ", RemainderWorkload< sw::universal::blockbinary<64> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128>  remainder     ", RemainderWorkload< sw::universal::blockbinary<128> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  remainder     ", RemainderWorkload< sw::universal::blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> remainder     ", RemainderWorkload< sw::universal::blockbinary<1024> >, NR_OPS / 16);

}

void TestBlockPerformanceOnAdd() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "ADDITION: blockbinary arithemetic performance as a function of size and BlockType" << endl;

	constexpr size_t NR_OPS = 32ull * 1024ull * 1024ull;

	PerformanceRunner("blockbinary<4,uint8>      add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>   add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>   add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>   add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>   add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024,uint8>   add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16>  add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint32>  add   ", AdditionSubtractionWorkload< sw::universal::blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestBlockPerformanceOnDiv() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "DIVISION: blockbinary arithemetic performance as a function of size and BlockType" << endl;

	constexpr size_t NR_OPS = 1024ull * 1024;
	PerformanceRunner("blockbinary<4,uint8>      div   ", DivisionWorkload< sw::universal::blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      div   ", DivisionWorkload< sw::universal::blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     div   ", DivisionWorkload< sw::universal::blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    div   ", DivisionWorkload< sw::universal::blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     div   ", DivisionWorkload< sw::universal::blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    div   ", DivisionWorkload< sw::universal::blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    div   ", DivisionWorkload< sw::universal::blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     div   ", DivisionWorkload< sw::universal::blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    div   ", DivisionWorkload< sw::universal::blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    div   ", DivisionWorkload< sw::universal::blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    div   ", DivisionWorkload< sw::universal::blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   div   ", DivisionWorkload< sw::universal::blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   div   ", DivisionWorkload< sw::universal::blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    div   ", DivisionWorkload< sw::universal::blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>   div   ", DivisionWorkload< sw::universal::blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>   div   ", DivisionWorkload< sw::universal::blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    div   ", DivisionWorkload< sw::universal::blockbinary<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>   div   ", DivisionWorkload< sw::universal::blockbinary<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>   div   ", DivisionWorkload< sw::universal::blockbinary<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024,uint8>   div   ", DivisionWorkload< sw::universal::blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16>  div   ", DivisionWorkload< sw::universal::blockbinary<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint32>  div   ", DivisionWorkload< sw::universal::blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestBlockPerformanceOnRem() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "REMAINDER: blockbinary arithemetic performance as a function of size and BlockType" << endl;

	constexpr size_t NR_OPS = 1024ull * 1024;
	PerformanceRunner("blockbinary<4,uint8>      rem   ", RemainderWorkload< sw::universal::blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      rem   ", RemainderWorkload< sw::universal::blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     rem   ", RemainderWorkload< sw::universal::blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    rem   ", RemainderWorkload< sw::universal::blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     rem   ", RemainderWorkload< sw::universal::blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    rem   ", RemainderWorkload< sw::universal::blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    rem   ", RemainderWorkload< sw::universal::blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     rem   ", RemainderWorkload< sw::universal::blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    rem   ", RemainderWorkload< sw::universal::blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    rem   ", RemainderWorkload< sw::universal::blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    rem   ", RemainderWorkload< sw::universal::blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   rem   ", RemainderWorkload< sw::universal::blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   rem   ", RemainderWorkload< sw::universal::blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    rem   ", RemainderWorkload< sw::universal::blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>   rem   ", RemainderWorkload< sw::universal::blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>   rem   ", RemainderWorkload< sw::universal::blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    rem   ", RemainderWorkload< sw::universal::blockbinary<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>   rem   ", RemainderWorkload< sw::universal::blockbinary<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>   rem   ", RemainderWorkload< sw::universal::blockbinary<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024,uint8>   rem   ", RemainderWorkload< sw::universal::blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16>  rem   ", RemainderWorkload< sw::universal::blockbinary<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint32>  rem   ", RemainderWorkload< sw::universal::blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestBlockPerformanceOnMul() {
	using namespace std;
	using namespace sw::universal;
	cout << endl << "MULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType" << endl;

	constexpr size_t NR_OPS = 512ull * 1024;
	PerformanceRunner("blockbinary<4,uint8>      mul   ", MultiplicationWorkload< sw::universal::blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      mul   ", MultiplicationWorkload< sw::universal::blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     mul   ", MultiplicationWorkload< sw::universal::blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     mul   ", MultiplicationWorkload< sw::universal::blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     mul   ", MultiplicationWorkload< sw::universal::blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   mul   ", MultiplicationWorkload< sw::universal::blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   mul   ", MultiplicationWorkload< sw::universal::blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<256, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<256,uint16>   mul   ", MultiplicationWorkload< sw::universal::blockbinary<256, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<256,uint32>   mul   ", MultiplicationWorkload< sw::universal::blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    mul   ", MultiplicationWorkload< sw::universal::blockbinary<512, uint8_t> >, NR_OPS / 512);
	PerformanceRunner("blockbinary<512,uint16>   mul   ", MultiplicationWorkload< sw::universal::blockbinary<512, uint16_t> >, NR_OPS / 256);
	PerformanceRunner("blockbinary<512,uint32>   mul   ", MultiplicationWorkload< sw::universal::blockbinary<512, uint32_t> >, NR_OPS / 128);
	PerformanceRunner("blockbinary<1024,uint8>   mul   ", MultiplicationWorkload< sw::universal::blockbinary<1024, uint8_t> >, NR_OPS / 1024);
	PerformanceRunner("blockbinary<1024,uint16>  mul   ", MultiplicationWorkload< sw::universal::blockbinary<1024, uint16_t> >, NR_OPS / 512);
	PerformanceRunner("blockbinary<1024,uint32>  mul   ", MultiplicationWorkload< sw::universal::blockbinary<1024, uint32_t> >, NR_OPS / 256);

}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "Integer operator performance benchmarking";

#if MANUAL_TESTING

	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	ShiftPerformanceWorkload< sw::universal::blockbinary<8, uint8_t> >(1);
	
	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;

	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	TestBlockPerformanceOnShift();
	TestBlockPerformanceOnAdd();
	TestBlockPerformanceOnMul();
	TestBlockPerformanceOnDiv();
	TestBlockPerformanceOnRem();

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

/*
ETLO
Date run : 2/25/2020
Processor: Intel Core i7-7500 CPU @ 2.70GHz, 2 cores, 4 threads, 15W mobile processor
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 1803, x64-based processor, OS build 17134.165

Integer operator performance benchmarking

Logical shift operator performance
blockbinary<16>   shifts            1000000 per       0.0013967sec -> 715 Mops/sec
blockbinary<32>   shifts            1000000 per       0.0063932sec -> 156 Mops/sec
blockbinary<64>   shifts            1000000 per       0.0210088sec ->  47 Mops/sec
blockbinary<128>  shifts             500000 per       0.0186187sec ->  26 Mops/sec
blockbinary<256>  shifts             250000 per       0.0265821sec ->   9 Mops/sec
blockbinary<512>  shifts             125000 per       0.0387919sec ->   3 Mops/sec
blockbinary<1024> shifts              62500 per       0.0252466sec ->   2 Mops/sec

Arithmetic operator performance
blockbinary<16>   add/subtract      1000000 per       0.0129235sec ->  77 Mops/sec
blockbinary<32>   add/subtract      1000000 per       0.0212104sec ->  47 Mops/sec
blockbinary<64>   add/subtract      1000000 per       0.0308646sec ->  32 Mops/sec
blockbinary<128>  add/subtract       500000 per       0.0303105sec ->  16 Mops/sec
blockbinary<256>  add/subtract       250000 per       0.0308254sec ->   8 Mops/sec
blockbinary<512>  add/subtract       125000 per       0.0354519sec ->   3 Mops/sec
blockbinary<1024> add/subtract        62500 per       0.0366506sec ->   1 Mops/sec
blockbinary<16>   division            32768 per       0.0022338sec ->  14 Mops/sec
blockbinary<32>   division            32768 per       0.0028381sec ->  11 Mops/sec
blockbinary<64>   division            16384 per       0.0024436sec ->   6 Mops/sec
blockbinary<128>  division             8192 per       0.0024895sec ->   3 Mops/sec
blockbinary<512>  division             4096 per       0.0060114sec -> 681 Kops/sec
blockbinary<1024> division             2048 per       0.0067702sec -> 302 Kops/sec
blockbinary<16>   remainder           32768 per       0.0025087sec ->  13 Mops/sec
blockbinary<32>   remainder           32768 per       0.0034877sec ->   9 Mops/sec
blockbinary<64>   remainder           16384 per       0.0031411sec ->   5 Mops/sec
blockbinary<128>  remainder            8192 per       0.0026535sec ->   3 Mops/sec
blockbinary<512>  remainder            4096 per       0.0071541sec -> 572 Kops/sec
blockbinary<1024> remainder            2048 per        0.006772sec -> 302 Kops/sec
blockbinary<16>   multiplication      32768 per       0.0013644sec ->  24 Mops/sec
blockbinary<32>   multiplication      16384 per       0.0027321sec ->   5 Mops/sec
blockbinary<64>   multiplication       8192 per       0.0051987sec ->   1 Mops/sec
blockbinary<128>  multiplication       4096 per       0.0163804sec -> 250 Kops/sec
blockbinary<512>  multiplication       2048 per        0.242991sec ->   8 Kops/sec
blockbinary<1024> multiplication       1024 per        0.283926sec ->   3 Kops/sec

Block size performance on logical shift operators
blockbinary<8,uint8>     shifts      1000000 per           3e-07sec ->   3 Tops/sec
blockbinary<16,uint8>    shifts      1000000 per       0.0016431sec -> 608 Mops/sec
blockbinary<16,uint16>   shifts      1000000 per           1e-07sec ->  10 Tops/sec
blockbinary<32,uint8>    shifts      1000000 per       0.0040908sec -> 244 Mops/sec
blockbinary<32,uint16>   shifts      1000000 per       0.0019056sec -> 524 Mops/sec
blockbinary<32,uint32>   shifts      1000000 per           2e-07sec ->   5 Tops/sec
blockbinary<64,uint8>    shifts      1000000 per       0.0130328sec ->  76 Mops/sec
blockbinary<64,uint16>   shifts      1000000 per       0.0053097sec -> 188 Mops/sec
blockbinary<64,uint32>   shifts      1000000 per       0.0007268sec ->   1 Gops/sec
blockbinary<128,uint8>   shifts       500000 per       0.0146397sec ->  34 Mops/sec
blockbinary<128,uint16>  shifts       500000 per       0.0050691sec ->  98 Mops/sec
blockbinary<128,uint32>  shifts       500000 per       0.0022174sec -> 225 Mops/sec
blockbinary<256,uint8>   shifts       250000 per       0.0245585sec ->  10 Mops/sec
blockbinary<256,uint16>  shifts       250000 per       0.0096754sec ->  25 Mops/sec
blockbinary<256,uint32>  shifts       250000 per       0.0028712sec ->  87 Mops/sec
blockbinary<512,uint8>   shifts       125000 per       0.0418973sec ->   2 Mops/sec
blockbinary<512,uint16>  shifts       125000 per       0.0353224sec ->   3 Mops/sec
blockbinary<512,uint32>  shifts       125000 per       0.0071748sec ->  17 Mops/sec
blockbinary<1024,uint8>  shifts        62500 per       0.0248754sec ->   2 Mops/sec
blockbinary<1024,uint16> shifts        62500 per       0.0177027sec ->   3 Mops/sec
blockbinary<1024,uint32> shifts        62500 per        0.003406sec ->  18 Mops/sec

block size performance
blockbinary<4,uint8>      add       1000000 per        0.001309sec -> 763 Mops/sec
blockbinary<8,uint8>      add       1000000 per       0.0010282sec -> 972 Mops/sec
blockbinary<16,uint8>     add       1000000 per       0.0125934sec ->  79 Mops/sec
blockbinary<16,uint16>    add       1000000 per       0.0009041sec ->   1 Gops/sec
blockbinary<32,uint8>     add       1000000 per        0.019086sec ->  52 Mops/sec
blockbinary<32,uint16>    add       1000000 per       0.0143736sec ->  69 Mops/sec
blockbinary<32,uint32>    add       1000000 per       0.0006061sec ->   1 Gops/sec
blockbinary<64,uint8>     add       1000000 per        0.030433sec ->  32 Mops/sec
blockbinary<64,uint16>    add       1000000 per       0.0186879sec ->  53 Mops/sec
blockbinary<64,uint32>    add       1000000 per        0.012867sec ->  77 Mops/sec
blockbinary<128,uint8>    add        500000 per       0.0304265sec ->  16 Mops/sec
blockbinary<128,uint16>   add        500000 per        0.015848sec ->  31 Mops/sec
blockbinary<128,uint32>   add        500000 per       0.0129498sec ->  38 Mops/sec
blockbinary<256,uint8>    add        250000 per       0.0419406sec ->   5 Mops/sec
blockbinary<256,uint16>   add        250000 per       0.0171904sec ->  14 Mops/sec
blockbinary<256,uint32>   add        250000 per       0.0079432sec ->  31 Mops/sec
blockbinary<512,uint8>    add        125000 per       0.0360764sec ->   3 Mops/sec
blockbinary<512,uint16>   add        125000 per       0.0168952sec ->   7 Mops/sec
blockbinary<512,uint32>   add        125000 per       0.0068201sec ->  18 Mops/sec
blockbinary<1024,uint8>   add         62500 per       0.0372607sec ->   1 Mops/sec
blockbinary<1024,uint16>  add         62500 per       0.0183361sec ->   3 Mops/sec
blockbinary<1024,uint32>  add         62500 per       0.0084381sec ->   7 Mops/sec

block size performance
blockbinary<4,uint8>      mul        500000 per        0.006492sec ->  77 Mops/sec
blockbinary<8,uint8>      mul        500000 per       0.0084224sec ->  59 Mops/sec
blockbinary<16,uint8>     mul        500000 per       0.0196726sec ->  25 Mops/sec
blockbinary<16,uint16>    mul        500000 per        0.009335sec ->  53 Mops/sec
blockbinary<32,uint8>     mul        500000 per       0.0685858sec ->   7 Mops/sec
blockbinary<32,uint16>    mul        500000 per        0.045748sec ->  10 Mops/sec
blockbinary<32,uint32>    mul        500000 per       0.0264499sec ->  18 Mops/sec
blockbinary<64,uint8>     mul        500000 per        0.310805sec ->   1 Mops/sec
blockbinary<64,uint16>    mul        500000 per        0.152077sec ->   3 Mops/sec
blockbinary<64,uint32>    mul        500000 per       0.0731226sec ->   6 Mops/sec
blockbinary<128,uint8>    mul        250000 per        0.758251sec -> 329 Kops/sec
blockbinary<128,uint16>   mul        250000 per         0.31448sec -> 794 Kops/sec
blockbinary<128,uint32>   mul        250000 per        0.147211sec ->   1 Mops/sec
blockbinary<256,uint8>    mul         62500 per         1.05118sec ->  59 Kops/sec
blockbinary<256,uint16>   mul        125000 per        0.755691sec -> 165 Kops/sec
blockbinary<256,uint32>   mul        125000 per        0.373575sec -> 334 Kops/sec
blockbinary<512,uint8>    mul         15625 per          1.8431sec ->   8 Kops/sec
blockbinary<512,uint16>   mul         31250 per         1.03459sec ->  30 Kops/sec
blockbinary<512,uint32>   mul         62500 per        0.760108sec ->  82 Kops/sec
blockbinary<1024,uint8>   mul          7812 per         2.26072sec ->   3 Kops/sec
blockbinary<1024,uint16>  mul         15625 per         3.68005sec ->   4 Kops/sec
blockbinary<1024,uint32>  mul         31250 per         2.08747sec ->  14 Kops/sec

block size performance
blockbinary<4,uint8>      div       1000000 per       0.0227702sec ->  43 Mops/sec
blockbinary<8,uint8>      div       1000000 per       0.0581431sec ->  17 Mops/sec
blockbinary<16,uint8>     div       1000000 per       0.0693781sec ->  14 Mops/sec
blockbinary<16,uint16>    div       1000000 per       0.0742125sec ->  13 Mops/sec
blockbinary<32,uint8>     div       1000000 per        0.088973sec ->  11 Mops/sec
blockbinary<32,uint16>    div       1000000 per       0.0700939sec ->  14 Mops/sec
blockbinary<32,uint32>    div       1000000 per       0.0728259sec ->  13 Mops/sec
blockbinary<64,uint8>     div       1000000 per        0.149565sec ->   6 Mops/sec
blockbinary<64,uint16>    div       1000000 per       0.0992803sec ->  10 Mops/sec
blockbinary<64,uint32>    div       1000000 per       0.0781339sec ->  12 Mops/sec
blockbinary<128,uint8>    div        500000 per        0.153718sec ->   3 Mops/sec
blockbinary<128,uint16>   div        500000 per       0.0874746sec ->   5 Mops/sec
blockbinary<128,uint32>   div        500000 per       0.0541529sec ->   9 Mops/sec
blockbinary<256,uint8>    div        250000 per        0.170179sec ->   1 Mops/sec
blockbinary<256,uint16>   div        250000 per       0.0835185sec ->   2 Mops/sec
blockbinary<256,uint32>   div        250000 per       0.0421212sec ->   5 Mops/sec
blockbinary<512,uint8>    div        125000 per        0.185005sec -> 675 Kops/sec
blockbinary<512,uint16>   div        125000 per       0.0944967sec ->   1 Mops/sec
blockbinary<512,uint32>   div        125000 per       0.0419085sec ->   2 Mops/sec
blockbinary<1024,uint8>   div         62500 per        0.191113sec -> 327 Kops/sec
blockbinary<1024,uint16>  div         62500 per       0.0971181sec -> 643 Kops/sec
blockbinary<1024,uint32>  div         62500 per       0.0517993sec ->   1 Mops/sec

block size performance
blockbinary<4,uint8>      rem       1000000 per       0.0276357sec ->  36 Mops/sec
blockbinary<8,uint8>      rem       1000000 per       0.0594847sec ->  16 Mops/sec
blockbinary<16,uint8>     rem       1000000 per       0.0680941sec ->  14 Mops/sec
blockbinary<16,uint16>    rem       1000000 per       0.0677176sec ->  14 Mops/sec
blockbinary<32,uint8>     rem       1000000 per       0.0971664sec ->  10 Mops/sec
blockbinary<32,uint16>    rem       1000000 per       0.0718622sec ->  13 Mops/sec
blockbinary<32,uint32>    rem       1000000 per        0.071596sec ->  13 Mops/sec
blockbinary<64,uint8>     rem       1000000 per        0.155253sec ->   6 Mops/sec
blockbinary<64,uint16>    rem       1000000 per       0.0999466sec ->  10 Mops/sec
blockbinary<64,uint32>    rem       1000000 per       0.0821133sec ->  12 Mops/sec
blockbinary<128,uint8>    rem        500000 per        0.159737sec ->   3 Mops/sec
blockbinary<128,uint16>   rem        500000 per       0.0863155sec ->   5 Mops/sec
blockbinary<128,uint32>   rem        500000 per       0.0596871sec ->   8 Mops/sec
blockbinary<256,uint8>    rem        250000 per        0.169751sec ->   1 Mops/sec
blockbinary<256,uint16>   rem        250000 per        0.093173sec ->   2 Mops/sec
blockbinary<256,uint32>   rem        250000 per       0.0412379sec ->   6 Mops/sec
blockbinary<512,uint8>    rem        125000 per        0.189013sec -> 661 Kops/sec
blockbinary<512,uint16>   rem        125000 per       0.0946558sec ->   1 Mops/sec
blockbinary<512,uint32>   rem        125000 per       0.0427583sec ->   2 Mops/sec
blockbinary<1024,uint8>   rem         62500 per        0.194026sec -> 322 Kops/sec
blockbinary<1024,uint16>  rem         62500 per       0.0972668sec -> 642 Kops/sec
blockbinary<1024,uint32>  rem         62500 per       0.0481189sec ->   1 Mops/sec
 */