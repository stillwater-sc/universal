//  performance.cpp : performance benchmarking for block binary number arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <string>
#include <chrono>

//#include <universal/integer/integer.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp> // ReportTestResult
#include <universal/verification/blockbinary_test_status.hpp>
#include <universal/benchmark/performance_runner.hpp>

namespace sw::universal::bb {

	// Specialized workloads for testing blockbinary performance
	// We need to specialize these because blockbinary doesn't have
	// a construct to consume a floating-point number, so both type
	// and value need to be specific to blockbinary test setup.

	// we isolate these specialized blockbinary performance workloads
	// into their own namespace, to avoid colliding with the generic versions.

	// blockbinary construction workload
	template<typename BlockbinaryConfiguration>
	void ConstructionPerformanceWorkload(size_t NR_OPS) {
		int positives{ 0 }, negatives{ 0 };
		for (size_t i = 0; i < NR_OPS; ++i) {
			BlockbinaryConfiguration a; // don't initialize with i as that is a conversion operation
			a.setbits(i);
			if (a.sign()) ++positives; else ++negatives;
		}
		if (positives == negatives) std::cout << "positives and negatives are identical (unlikely event to select)\n";
	}

	// blockbinary workload for testing shift operations on a given number system type that supports operator>> and operator<<
	template<typename BlockbinaryConfiguration>
	void ShiftPerformanceWorkload(size_t NR_OPS) {
		BlockbinaryConfiguration a;
		a.setbits(0xFFFF'FFFF'FFFF'FFFFull);
		for (size_t i = 0; i < NR_OPS; ++i) {
			a >>= 13;
			a <<= 37;
		}
	}

	// blockbinary set of adds and subtracts for a given number system type
	template<typename BlockbinaryConfiguration>
	void AdditionSubtractionWorkload(size_t NR_OPS) {
		std::vector<BlockbinaryConfiguration> data = { 3, -3 };
		BlockbinaryConfiguration a{ 0 }, b{ 1 };
		for (size_t i = 1; i < NR_OPS; ++i) {
			a = data[i % 2];
			b = b + a;
		}
		if (b == a) {
			std::cout << "dummy case to fool the optimizer\n";
		}
	}

	// blockbinary set of multiplies for a given number system type
	template<typename BlockbinaryConfiguration>
	void MultiplicationWorkload(size_t NR_OPS) {
		std::vector<BlockbinaryConfiguration> data = { 1, -1 };
		BlockbinaryConfiguration a{ 0 }, b{ 3 };
		for (size_t i = 1; i < NR_OPS; ++i) {
			a = data[i % 2];
			b = b * a;
		}
		if (b == a) {
			std::cout << "dummy case to fool the optimizer\n";
		}
	}

	// blockbinary set of divides for a given number system type
	template<typename BlockbinaryConfiguration>
	void DivisionWorkload(size_t NR_OPS) {
		std::vector<BlockbinaryConfiguration> data = { 1, -1 };
		BlockbinaryConfiguration a{ 0 }, b{ 3 };
		for (size_t i = 1; i < NR_OPS; ++i) {
			a = data[i % 2];
			b = b / a;
		}
		if (b == a) {
			std::cout << "dummy case to fool the optimizer\n";
		}
	}

	// blockbinary set of remainder calculations for a given number system type that supports the % operator
	template<typename BlockbinaryConfiguration>
	void RemainderWorkload(size_t NR_OPS) {
		std::vector<BlockbinaryConfiguration> data = { 1, -1 };
		BlockbinaryConfiguration a{ 0 }, b{ 3 };
		for (size_t i = 0; i < NR_OPS; ++i) {
			a = data[i % 2];
			b = b % a;
		}
		if (b == a) {
			std::cout << "dummy case to fool the optimizer\n";
		}
	}
} // namespace sw::universal::bb

// test construction peformance
void TestBlockPerformanceOnConstruction() {
	using namespace sw::universal;
	std::cout << "\nConstruction performance\n";

	constexpr size_t NR_OPS = 1024ull * 1024ull + 1;

	PerformanceRunner("blockbinary<8>    construction  ", bb::ConstructionPerformanceWorkload< blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16>   construction  ", bb::ConstructionPerformanceWorkload< blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   construction  ", bb::ConstructionPerformanceWorkload< blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   construction  ", bb::ConstructionPerformanceWorkload< blockbinary<64, uint64_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128>  construction  ", bb::ConstructionPerformanceWorkload< blockbinary<128, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<256>  construction  ", bb::ConstructionPerformanceWorkload< blockbinary<256, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<512>  construction  ", bb::ConstructionPerformanceWorkload< blockbinary<512, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<1024> construction  ", bb::ConstructionPerformanceWorkload< blockbinary<1024, uint32_t> >, NR_OPS);
}

// test performance of shift operator on blockbinary<> class
void TestShiftOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nLogical shift operator performance\n";

	constexpr size_t NR_OPS = 1024ull * 1024ull;

	PerformanceRunner("blockbinary<16>   shifts        ", bb::ShiftPerformanceWorkload< blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   shifts        ", bb::ShiftPerformanceWorkload< blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   shifts        ", bb::ShiftPerformanceWorkload< blockbinary<64> >, NR_OPS);
	PerformanceRunner("blockbinary<128>  shifts        ", bb::ShiftPerformanceWorkload< blockbinary<128> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256>  shifts        ", bb::ShiftPerformanceWorkload< blockbinary<256> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  shifts        ", bb::ShiftPerformanceWorkload< blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> shifts        ", bb::ShiftPerformanceWorkload< blockbinary<1024> >, NR_OPS / 16);
}

void TestBlockPerformanceOnShift() {
	using namespace sw::universal;
	std::cout << "\nBlock size performance on logical shift operators\n";

	constexpr size_t NR_OPS = 1024ull * 1024ull;

	PerformanceRunner("blockbinary<8,uint8>     shifts  ", bb::ShiftPerformanceWorkload< blockbinary<8, uint8_t> >, NR_OPS);

	PerformanceRunner("blockbinary<16,uint8>    shifts  ", bb::ShiftPerformanceWorkload< blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<16, uint16_t> >, NR_OPS);

	PerformanceRunner("blockbinary<32,uint8>    shifts  ", bb::ShiftPerformanceWorkload< blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<32, uint32_t> >, NR_OPS);

	PerformanceRunner("blockbinary<64,uint8>    shifts  ", bb::ShiftPerformanceWorkload< blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<64, uint32_t> >, NR_OPS);

	PerformanceRunner("blockbinary<128,uint8>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>  shifts  ", bb::ShiftPerformanceWorkload< blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>  shifts  ", bb::ShiftPerformanceWorkload< blockbinary<128, uint32_t> >, NR_OPS / 2);

	PerformanceRunner("blockbinary<256,uint8>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>  shifts  ", bb::ShiftPerformanceWorkload< blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>  shifts  ", bb::ShiftPerformanceWorkload< blockbinary<256, uint32_t> >, NR_OPS / 4);

	PerformanceRunner("blockbinary<512,uint8>   shifts  ", bb::ShiftPerformanceWorkload< blockbinary<1024, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>  shifts  ", bb::ShiftPerformanceWorkload< blockbinary<1024, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>  shifts  ", bb::ShiftPerformanceWorkload< blockbinary<1024, uint32_t> >, NR_OPS / 8);

	PerformanceRunner("blockbinary<1024,uint8>  shifts  ", bb::ShiftPerformanceWorkload< blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16> shifts  ", bb::ShiftPerformanceWorkload< blockbinary<1024, uint16_t> >, NR_OPS /16);
	PerformanceRunner("blockbinary<1024,uint32> shifts  ", bb::ShiftPerformanceWorkload< blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nArithmetic operator performance\n";

	size_t NR_OPS = 1024ull * 1024ull * 2ull;

	PerformanceRunner("blockbinary<16>   add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<64> >, NR_OPS);
	PerformanceRunner("blockbinary<128>  add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<128> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256>  add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<256> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<1024> >, NR_OPS / 16);

	NR_OPS = 1024ull * 1024ull;
	PerformanceRunner("blockbinary<16>   multiplication", bb::MultiplicationWorkload< blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   multiplication", bb::MultiplicationWorkload< blockbinary<32> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<64>   multiplication", bb::MultiplicationWorkload< blockbinary<64> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<128>  multiplication", bb::MultiplicationWorkload< blockbinary<128> >, NR_OPS / 64);
	PerformanceRunner("blockbinary<512>  multiplication", bb::MultiplicationWorkload< blockbinary<512> >, NR_OPS / 512);     // TODO: why is this so slow?
	PerformanceRunner("blockbinary<1024> multiplication", bb::MultiplicationWorkload< blockbinary<1024> >, NR_OPS / 1024);   // TODO: why is this so slow?

	NR_OPS = 1024ull * 512ull;
	PerformanceRunner("blockbinary<16>   division      ", bb::DivisionWorkload< blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   division      ", bb::DivisionWorkload< blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   division      ", bb::DivisionWorkload< blockbinary<64> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128>  division      ", bb::DivisionWorkload< blockbinary<128> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  division      ", bb::DivisionWorkload< blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> division      ", bb::DivisionWorkload< blockbinary<1024> >, NR_OPS / 16);

	NR_OPS = 1024ull * 512ull;
	PerformanceRunner("blockbinary<16>   remainder     ", bb::RemainderWorkload< blockbinary<16> >, NR_OPS);
	PerformanceRunner("blockbinary<32>   remainder     ", bb::RemainderWorkload< blockbinary<32> >, NR_OPS);
	PerformanceRunner("blockbinary<64>   remainder     ", bb::RemainderWorkload< blockbinary<64> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128>  remainder     ", bb::RemainderWorkload< blockbinary<128> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512>  remainder     ", bb::RemainderWorkload< blockbinary<512> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024> remainder     ", bb::RemainderWorkload< blockbinary<1024> >, NR_OPS / 16);
}

void TestBlockPerformanceOnAdd() {
	using namespace sw::universal;
	std::cout << "\nADDITION: blockbinary arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 2ull * 1024ull * 1024ull;

	PerformanceRunner("blockbinary<4,uint8>      add   ", bb::AdditionSubtractionWorkload< blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      add   ", bb::AdditionSubtractionWorkload< blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     add   ", bb::AdditionSubtractionWorkload< blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    add   ", bb::AdditionSubtractionWorkload< blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     add   ", bb::AdditionSubtractionWorkload< blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    add   ", bb::AdditionSubtractionWorkload< blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    add   ", bb::AdditionSubtractionWorkload< blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     add   ", bb::AdditionSubtractionWorkload< blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    add   ", bb::AdditionSubtractionWorkload< blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    add   ", bb::AdditionSubtractionWorkload< blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    add   ", bb::AdditionSubtractionWorkload< blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   add   ", bb::AdditionSubtractionWorkload< blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   add   ", bb::AdditionSubtractionWorkload< blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    add   ", bb::AdditionSubtractionWorkload< blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>   add   ", bb::AdditionSubtractionWorkload< blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>   add   ", bb::AdditionSubtractionWorkload< blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    add   ", bb::AdditionSubtractionWorkload< blockbinary<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>   add   ", bb::AdditionSubtractionWorkload< blockbinary<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>   add   ", bb::AdditionSubtractionWorkload< blockbinary<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024,uint8>   add   ", bb::AdditionSubtractionWorkload< blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16>  add   ", bb::AdditionSubtractionWorkload< blockbinary<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint32>  add   ", bb::AdditionSubtractionWorkload< blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestBlockPerformanceOnDiv() {
	using namespace sw::universal;
	std::cout << "\nDIVISION: blockbinary arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 1024ull * 1024;
	PerformanceRunner("blockbinary<4,uint8>      div   ", bb::DivisionWorkload< blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      div   ", bb::DivisionWorkload< blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     div   ", bb::DivisionWorkload< blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    div   ", bb::DivisionWorkload< blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     div   ", bb::DivisionWorkload< blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    div   ", bb::DivisionWorkload< blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    div   ", bb::DivisionWorkload< blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     div   ", bb::DivisionWorkload< blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    div   ", bb::DivisionWorkload< blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    div   ", bb::DivisionWorkload< blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint64>    div   ", bb::DivisionWorkload< blockbinary<64, uint64_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    div   ", bb::DivisionWorkload< blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   div   ", bb::DivisionWorkload< blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   div   ", bb::DivisionWorkload< blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    div   ", bb::DivisionWorkload< blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>   div   ", bb::DivisionWorkload< blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>   div   ", bb::DivisionWorkload< blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    div   ", bb::DivisionWorkload< blockbinary<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>   div   ", bb::DivisionWorkload< blockbinary<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>   div   ", bb::DivisionWorkload< blockbinary<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024,uint8>   div   ", bb::DivisionWorkload< blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16>  div   ", bb::DivisionWorkload< blockbinary<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint32>  div   ", bb::DivisionWorkload< blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestBlockPerformanceOnRem() {
	using namespace sw::universal;
	std::cout << "\nREMAINDER: blockbinary arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 1024ull * 1024;
	PerformanceRunner("blockbinary<4,uint8>      rem   ", bb::RemainderWorkload< blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      rem   ", bb::RemainderWorkload< blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     rem   ", bb::RemainderWorkload< blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    rem   ", bb::RemainderWorkload< blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     rem   ", bb::RemainderWorkload< blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    rem   ", bb::RemainderWorkload< blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    rem   ", bb::RemainderWorkload< blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     rem   ", bb::RemainderWorkload< blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    rem   ", bb::RemainderWorkload< blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    rem   ", bb::RemainderWorkload< blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint64>    rem   ", bb::RemainderWorkload< blockbinary<64, uint64_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    rem   ", bb::RemainderWorkload< blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   rem   ", bb::RemainderWorkload< blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   rem   ", bb::RemainderWorkload< blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    rem   ", bb::RemainderWorkload< blockbinary<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint16>   rem   ", bb::RemainderWorkload< blockbinary<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<256,uint32>   rem   ", bb::RemainderWorkload< blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    rem   ", bb::RemainderWorkload< blockbinary<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint16>   rem   ", bb::RemainderWorkload< blockbinary<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<512,uint32>   rem   ", bb::RemainderWorkload< blockbinary<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<1024,uint8>   rem   ", bb::RemainderWorkload< blockbinary<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint16>  rem   ", bb::RemainderWorkload< blockbinary<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<1024,uint32>  rem   ", bb::RemainderWorkload< blockbinary<1024, uint32_t> >, NR_OPS / 16);
}

void TestBlockPerformanceOnMul() {
	using namespace sw::universal;
	std::cout << "\nMULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 512ull * 1024;
	PerformanceRunner("blockbinary<4,uint8>      mul   ", bb::MultiplicationWorkload< blockbinary<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<8,uint8>      mul   ", bb::MultiplicationWorkload< blockbinary<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint8>     mul   ", bb::MultiplicationWorkload< blockbinary<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<16,uint16>    mul   ", bb::MultiplicationWorkload< blockbinary<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint8>     mul   ", bb::MultiplicationWorkload< blockbinary<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint16>    mul   ", bb::MultiplicationWorkload< blockbinary<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<32,uint32>    mul   ", bb::MultiplicationWorkload< blockbinary<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint8>     mul   ", bb::MultiplicationWorkload< blockbinary<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint16>    mul   ", bb::MultiplicationWorkload< blockbinary<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint32>    mul   ", bb::MultiplicationWorkload< blockbinary<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockbinary<64,uint64>    mul   ", bb::MultiplicationWorkload< blockbinary<64, uint64_t> >, NR_OPS);
	PerformanceRunner("blockbinary<128,uint8>    mul   ", bb::MultiplicationWorkload< blockbinary<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint16>   mul   ", bb::MultiplicationWorkload< blockbinary<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<128,uint32>   mul   ", bb::MultiplicationWorkload< blockbinary<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockbinary<256,uint8>    mul   ", bb::MultiplicationWorkload< blockbinary<256, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockbinary<256,uint16>   mul   ", bb::MultiplicationWorkload< blockbinary<256, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockbinary<256,uint32>   mul   ", bb::MultiplicationWorkload< blockbinary<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockbinary<512,uint8>    mul   ", bb::MultiplicationWorkload< blockbinary<512, uint8_t> >, NR_OPS / 512);
	PerformanceRunner("blockbinary<512,uint16>   mul   ", bb::MultiplicationWorkload< blockbinary<512, uint16_t> >, NR_OPS / 256);
	PerformanceRunner("blockbinary<512,uint32>   mul   ", bb::MultiplicationWorkload< blockbinary<512, uint32_t> >, NR_OPS / 128);
	PerformanceRunner("blockbinary<1024,uint8>   mul   ", bb::MultiplicationWorkload< blockbinary<1024, uint8_t> >, NR_OPS / 1024);
	PerformanceRunner("blockbinary<1024,uint16>  mul   ", bb::MultiplicationWorkload< blockbinary<1024, uint16_t> >, NR_OPS / 512);
	PerformanceRunner("blockbinary<1024,uint32>  mul   ", bb::MultiplicationWorkload< blockbinary<1024, uint32_t> >, NR_OPS / 256);
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite          = "blockbinary operator performance benchmarking";
	std::string test_tag            = "blockbinary performance";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	PerformanceRunner("blockbinary<1024> add/subtract  ", bb::AdditionSubtractionWorkload< blockbinary<1024> >, 1024*1024);

	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	ShiftPerformanceWorkload< sw::universal::blockbinary<8, uint8_t> >(1);
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#if REGRESSION_LEVEL_1
	// benchmarking tests are LEVEL_4
	TestShiftOperatorPerformance();
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4

	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	TestBlockPerformanceOnConstruction();
	TestBlockPerformanceOnShift();
	TestBlockPerformanceOnAdd();
	TestBlockPerformanceOnMul();
	TestBlockPerformanceOnDiv();
	TestBlockPerformanceOnRem();

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

Blockbinary operator performance benchmarking

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

/*
ETLO
Date run : 03/01/2021
Processor: Intel Core i7-9850H CPU @ 2.60GHz, 6 cores, 12 threads, 15W mobile processor
Cache    : L1 384KB, L2 1.5MB, L3 12.0MB
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 2004, x64-based processor, OS build 19041.804
BLOCKBINARY operator performance benchmarking

Logical shift operator performance
blockbinary<16>   shifts            1048576 per       0.0151422sec ->  69 Mops/sec
blockbinary<32>   shifts            1048576 per       0.0169131sec ->  61 Mops/sec
blockbinary<64>   shifts            1048576 per         0.02377sec ->  44 Mops/sec
blockbinary<128>  shifts             524288 per       0.0170116sec ->  30 Mops/sec
blockbinary<256>  shifts             262144 per        0.023649sec ->  11 Mops/sec
blockbinary<512>  shifts             131072 per        0.017104sec ->   7 Mops/sec
blockbinary<1024> shifts              65536 per       0.0168191sec ->   3 Mops/sec

Arithmetic operator performance
blockbinary<16>   add/subtract      2097152 per       0.0239446sec ->  87 Mops/sec
blockbinary<32>   add/subtract      2097152 per       0.0317233sec ->  66 Mops/sec
blockbinary<64>   add/subtract      2097152 per       0.0522965sec ->  40 Mops/sec
blockbinary<128>  add/subtract      1048576 per       0.0609575sec ->  17 Mops/sec
blockbinary<256>  add/subtract       524288 per       0.0612585sec ->   8 Mops/sec
blockbinary<512>  add/subtract       262144 per       0.0649979sec ->   4 Mops/sec
blockbinary<1024> add/subtract       131072 per        0.065752sec ->   1 Mops/sec
blockbinary<16>   multiplication    1048576 per       0.0294896sec ->  35 Mops/sec
blockbinary<32>   multiplication     524288 per       0.0524751sec ->   9 Mops/sec
blockbinary<64>   multiplication     262144 per        0.121624sec ->   2 Mops/sec
blockbinary<128>  multiplication      16384 per       0.0313895sec -> 521 Kops/sec
blockbinary<512>  multiplication       2048 per       0.0673462sec ->  30 Kops/sec
blockbinary<1024> multiplication       1024 per       0.0913936sec ->  11 Kops/sec
blockbinary<16>   division           524288 per       0.0210676sec ->  24 Mops/sec
blockbinary<32>   division           524288 per       0.0326344sec ->  16 Mops/sec
blockbinary<64>   division           262144 per       0.0207816sec ->  12 Mops/sec
blockbinary<128>  division           131072 per       0.0166951sec ->   7 Mops/sec
blockbinary<512>  division            65536 per       0.0321721sec ->   2 Mops/sec
blockbinary<1024> division            32768 per       0.0352204sec -> 930 Kops/sec
blockbinary<16>   remainder          524288 per       0.0211667sec ->  24 Mops/sec
blockbinary<32>   remainder          524288 per       0.0262438sec ->  19 Mops/sec
blockbinary<64>   remainder          262144 per       0.0208688sec ->  12 Mops/sec
blockbinary<128>  remainder          131072 per       0.0150966sec ->   8 Mops/sec
blockbinary<512>  remainder           65536 per       0.0338635sec ->   1 Mops/sec
blockbinary<1024> remainder           32768 per       0.0372232sec -> 880 Kops/sec

Construction performance
blockbinary<8>    construction      1048577 per       0.0006647sec ->   1 Gops/sec
blockbinary<16>   construction      1048577 per       0.0008141sec ->   1 Gops/sec
blockbinary<32>   construction      1048577 per       0.0006457sec ->   1 Gops/sec
blockbinary<64>   construction      1048577 per       0.0005499sec ->   1 Gops/sec
blockbinary<128>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockbinary<256>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockbinary<512>  construction      1048577 per       0.0060603sec -> 173 Mops/sec
blockbinary<1024> construction      1048577 per       0.0093635sec -> 111 Mops/sec

Block size performance on logical shift operators
blockbinary<8,uint8>     shifts      1048576 per           2e-07sec ->   5 Tops/sec
blockbinary<16,uint8>    shifts      1048576 per        0.014898sec ->  70 Mops/sec
blockbinary<16,uint16>   shifts      1048576 per       0.0141619sec ->  74 Mops/sec
blockbinary<32,uint8>    shifts      1048576 per       0.0177451sec ->  59 Mops/sec
blockbinary<32,uint16>   shifts      1048576 per       0.0154789sec ->  67 Mops/sec
blockbinary<32,uint32>   shifts      1048576 per       0.0138923sec ->  75 Mops/sec
blockbinary<64,uint8>    shifts      1048576 per       0.0319717sec ->  32 Mops/sec
blockbinary<64,uint16>   shifts      1048576 per       0.0178939sec ->  58 Mops/sec
blockbinary<64,uint32>   shifts      1048576 per       0.0148067sec ->  70 Mops/sec
blockbinary<128,uint8>   shifts       524288 per       0.0172914sec ->  30 Mops/sec
blockbinary<128,uint16>  shifts       524288 per         0.01156sec ->  45 Mops/sec
blockbinary<128,uint32>  shifts       524288 per       0.0087054sec ->  60 Mops/sec
blockbinary<256,uint8>   shifts       262144 per       0.0236265sec ->  11 Mops/sec
blockbinary<256,uint16>  shifts       262144 per       0.0079614sec ->  32 Mops/sec
blockbinary<256,uint32>  shifts       262144 per        0.005102sec ->  51 Mops/sec
blockbinary<512,uint8>   shifts       131072 per       0.0326855sec ->   4 Mops/sec
blockbinary<512,uint16>  shifts       131072 per       0.0179826sec ->   7 Mops/sec
blockbinary<512,uint32>  shifts       131072 per       0.0083022sec ->  15 Mops/sec
blockbinary<1024,uint8>  shifts        65536 per       0.0165172sec ->   3 Mops/sec
blockbinary<1024,uint16> shifts        65536 per       0.0083414sec ->   7 Mops/sec
blockbinary<1024,uint32> shifts        65536 per       0.0043763sec ->  14 Mops/sec

ADDITION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      add       2097152 per       0.0005575sec ->   3 Gops/sec
blockbinary<8,uint8>      add       2097152 per               0sec ->   0  ops/sec
blockbinary<16,uint8>     add       2097152 per       0.0228691sec ->  91 Mops/sec
blockbinary<16,uint16>    add       2097152 per           1e-07sec ->  20 Tops/sec
blockbinary<32,uint8>     add       2097152 per       0.0321724sec ->  65 Mops/sec
blockbinary<32,uint16>    add       2097152 per       0.0238562sec ->  87 Mops/sec
blockbinary<32,uint32>    add       2097152 per           1e-07sec ->  20 Tops/sec
blockbinary<64,uint8>     add       2097152 per       0.0522177sec ->  40 Mops/sec
blockbinary<64,uint16>    add       2097152 per       0.0314089sec ->  66 Mops/sec
blockbinary<64,uint32>    add       2097152 per       0.0240764sec ->  87 Mops/sec
blockbinary<128,uint8>    add       1048576 per        0.061275sec ->  17 Mops/sec
blockbinary<128,uint16>   add       1048576 per       0.0229437sec ->  45 Mops/sec
blockbinary<128,uint32>   add       1048576 per       0.0113588sec ->  92 Mops/sec
blockbinary<256,uint8>    add        524288 per       0.0584427sec ->   8 Mops/sec
blockbinary<256,uint16>   add        524288 per       0.0260387sec ->  20 Mops/sec
blockbinary<256,uint32>   add        524288 per       0.0121753sec ->  43 Mops/sec
blockbinary<512,uint8>    add        262144 per        0.063064sec ->   4 Mops/sec
blockbinary<512,uint16>   add        262144 per       0.0289259sec ->   9 Mops/sec
blockbinary<512,uint32>   add        262144 per       0.0135417sec ->  19 Mops/sec
blockbinary<1024,uint8>   add        131072 per        0.068664sec ->   1 Mops/sec
blockbinary<1024,uint16>  add        131072 per       0.0332106sec ->   3 Mops/sec
blockbinary<1024,uint32>  add        131072 per       0.0153358sec ->   8 Mops/sec

MULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      mul        524288 per       0.0021491sec -> 243 Mops/sec
blockbinary<8,uint8>      mul        524288 per       0.0048791sec -> 107 Mops/sec
blockbinary<16,uint8>     mul        524288 per       0.0147906sec ->  35 Mops/sec
blockbinary<16,uint16>    mul        524288 per       0.0104389sec ->  50 Mops/sec
blockbinary<32,uint8>     mul        524288 per       0.0529245sec ->   9 Mops/sec
blockbinary<32,uint16>    mul        524288 per       0.0291708sec ->  17 Mops/sec
blockbinary<32,uint32>    mul        524288 per       0.0145993sec ->  35 Mops/sec
blockbinary<64,uint8>     mul        524288 per        0.240083sec ->   2 Mops/sec
blockbinary<64,uint16>    mul        524288 per        0.114896sec ->   4 Mops/sec
blockbinary<64,uint32>    mul        524288 per       0.0577882sec ->   9 Mops/sec
blockbinary<128,uint8>    mul        262144 per        0.501657sec -> 522 Kops/sec
blockbinary<128,uint16>   mul        262144 per        0.186017sec ->   1 Mops/sec
blockbinary<128,uint32>   mul        262144 per       0.0887587sec ->   2 Mops/sec
blockbinary<256,uint8>    mul         32768 per        0.305953sec -> 107 Kops/sec
blockbinary<256,uint16>   mul         65536 per        0.188149sec -> 348 Kops/sec
blockbinary<256,uint32>   mul        131072 per        0.135927sec -> 964 Kops/sec
blockbinary<512,uint8>    mul          1024 per       0.0329513sec ->  31 Kops/sec
blockbinary<512,uint16>   mul          2048 per       0.0342709sec ->  59 Kops/sec
blockbinary<512,uint32>   mul          4096 per       0.0206824sec -> 198 Kops/sec
blockbinary<1024,uint8>   mul           512 per       0.0422902sec ->  12 Kops/sec
blockbinary<1024,uint16>  mul          1024 per       0.0618386sec ->  16 Kops/sec
blockbinary<1024,uint32>  mul          2048 per       0.0635144sec ->  32 Kops/sec

DIVISION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      div       1048576 per       0.0174991sec ->  59 Mops/sec
blockbinary<8,uint8>      div       1048576 per       0.0276323sec ->  37 Mops/sec
blockbinary<16,uint8>     div       1048576 per       0.0426758sec ->  24 Mops/sec
blockbinary<16,uint16>    div       1048576 per       0.0421559sec ->  24 Mops/sec
blockbinary<32,uint8>     div       1048576 per       0.0528577sec ->  19 Mops/sec
blockbinary<32,uint16>    div       1048576 per       0.0422172sec ->  24 Mops/sec
blockbinary<32,uint32>    div       1048576 per       0.0424791sec ->  24 Mops/sec
blockbinary<64,uint8>     div       1048576 per       0.0832279sec ->  12 Mops/sec
blockbinary<64,uint16>    div       1048576 per       0.0575975sec ->  18 Mops/sec
blockbinary<64,uint32>    div       1048576 per       0.0533793sec ->  19 Mops/sec
blockbinary<128,uint8>    div        524288 per       0.0579575sec ->   9 Mops/sec
blockbinary<128,uint16>   div        524288 per       0.0287337sec ->  18 Mops/sec
blockbinary<128,uint32>   div        524288 per       0.0215491sec ->  24 Mops/sec
blockbinary<256,uint8>    div        262144 per       0.0597042sec ->   4 Mops/sec
blockbinary<256,uint16>   div        262144 per       0.0312451sec ->   8 Mops/sec
blockbinary<256,uint32>   div        262144 per       0.0148073sec ->  17 Mops/sec
blockbinary<512,uint8>    div        131072 per       0.0641508sec ->   2 Mops/sec
blockbinary<512,uint16>   div        131072 per       0.0325873sec ->   4 Mops/sec
blockbinary<512,uint32>   div        131072 per       0.0168981sec ->   7 Mops/sec
blockbinary<1024,uint8>   div         65536 per       0.0724597sec -> 904 Kops/sec
blockbinary<1024,uint16>  div         65536 per       0.0375644sec ->   1 Mops/sec
blockbinary<1024,uint32>  div         65536 per        0.017201sec ->   3 Mops/sec

REMAINDER: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      rem       1048576 per       0.0174541sec ->  60 Mops/sec
blockbinary<8,uint8>      rem       1048576 per       0.0279139sec ->  37 Mops/sec
blockbinary<16,uint8>     rem       1048576 per       0.0430577sec ->  24 Mops/sec
blockbinary<16,uint16>    rem       1048576 per       0.0418485sec ->  25 Mops/sec
blockbinary<32,uint8>     rem       1048576 per       0.0523426sec ->  20 Mops/sec
blockbinary<32,uint16>    rem       1048576 per       0.0417373sec ->  25 Mops/sec
blockbinary<32,uint32>    rem       1048576 per       0.0427313sec ->  24 Mops/sec
blockbinary<64,uint8>     rem       1048576 per       0.0834139sec ->  12 Mops/sec
blockbinary<64,uint16>    rem       1048576 per       0.0584555sec ->  17 Mops/sec
blockbinary<64,uint32>    rem       1048576 per       0.0552587sec ->  18 Mops/sec
blockbinary<128,uint8>    rem        524288 per       0.0585331sec ->   8 Mops/sec
blockbinary<128,uint16>   rem        524288 per        0.029061sec ->  18 Mops/sec
blockbinary<128,uint32>   rem        524288 per       0.0209504sec ->  25 Mops/sec
blockbinary<256,uint8>    rem        262144 per       0.0592536sec ->   4 Mops/sec
blockbinary<256,uint16>   rem        262144 per        0.031582sec ->   8 Mops/sec
blockbinary<256,uint32>   rem        262144 per       0.0144546sec ->  18 Mops/sec
blockbinary<512,uint8>    rem        131072 per       0.0645029sec ->   2 Mops/sec
blockbinary<512,uint16>   rem        131072 per       0.0332093sec ->   3 Mops/sec
blockbinary<512,uint32>   rem        131072 per       0.0172867sec ->   7 Mops/sec
blockbinary<1024,uint8>   rem         65536 per       0.0726507sec -> 902 Kops/sec
blockbinary<1024,uint16>  rem         65536 per       0.0391768sec ->   1 Mops/sec
blockbinary<1024,uint32>  rem         65536 per       0.0170348sec ->   3 Mops/sec
*/

/*
ETLO
Date run : 10/19/2021
Processor: Intel Core i7-10700 CPU @ 2.90GHz, 8 cores, 16 threads, 15W mobile processor
Cache    : L1 512KB, L2 2.0MB, L3 16.0MB
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 2004, x64-based processor, OS build 19041.1288

NOTE     : the arithmetic workload routines have been modified, so do not compare
           these results directly to previous results. We have modified them to
		   avoid the optimizer from removing work. Still some cases that need addressing
		   but better than before.

blockbinary operator performance benchmarking

Logical shift operator performance
blockbinary<16>   shifts            1048576 per       0.0107457sec ->  97 Mops/sec
blockbinary<32>   shifts            1048576 per       0.0128502sec ->  81 Mops/sec
blockbinary<64>   shifts            1048576 per       0.0190326sec ->  55 Mops/sec
blockbinary<128>  shifts             524288 per       0.0139388sec ->  37 Mops/sec
blockbinary<256>  shifts             262144 per       0.0169785sec ->  15 Mops/sec
blockbinary<512>  shifts             131072 per       0.0151063sec ->   8 Mops/sec
blockbinary<1024> shifts              65536 per       0.0142443sec ->   4 Mops/sec

Arithmetic operator performance
blockbinary<16>   add/subtract      2097152 per       0.0110639sec -> 189 Mops/sec
blockbinary<32>   add/subtract      2097152 per       0.0161362sec -> 129 Mops/sec
blockbinary<64>   add/subtract      2097152 per       0.0252336sec ->  83 Mops/sec
blockbinary<128>  add/subtract      1048576 per       0.0220567sec ->  47 Mops/sec
blockbinary<256>  add/subtract       524288 per       0.0161617sec ->  32 Mops/sec
blockbinary<512>  add/subtract       262144 per        0.017403sec ->  15 Mops/sec
blockbinary<1024> add/subtract       131072 per       0.0182356sec ->   7 Mops/sec
blockbinary<16>   multiplication    1048576 per        0.021832sec ->  48 Mops/sec
blockbinary<32>   multiplication     524288 per       0.0354722sec ->  14 Mops/sec
blockbinary<64>   multiplication     262144 per       0.0675826sec ->   3 Mops/sec
blockbinary<128>  multiplication      16384 per       0.0272582sec -> 601 Kops/sec
blockbinary<512>  multiplication       2048 per       0.0839554sec ->  24 Kops/sec
blockbinary<1024> multiplication       1024 per        0.134298sec ->   7 Kops/sec
blockbinary<16>   division           524288 per       0.0272064sec ->  19 Mops/sec
blockbinary<32>   division           524288 per        0.032978sec ->  15 Mops/sec
blockbinary<64>   division           262144 per       0.0315587sec ->   8 Mops/sec
blockbinary<128>  division           131072 per       0.0361727sec ->   3 Mops/sec
blockbinary<512>  division            65536 per       0.0739774sec -> 885 Kops/sec
blockbinary<1024> division            32768 per       0.0764698sec -> 428 Kops/sec
blockbinary<16>   remainder          524288 per       0.0186509sec ->  28 Mops/sec
blockbinary<32>   remainder          524288 per       0.0218494sec ->  23 Mops/sec
blockbinary<64>   remainder          262144 per       0.0170074sec ->  15 Mops/sec
blockbinary<128>  remainder          131072 per       0.0121151sec ->  10 Mops/sec
blockbinary<512>  remainder           65536 per       0.0272976sec ->   2 Mops/sec
blockbinary<1024> remainder           32768 per       0.0296483sec ->   1 Mops/sec

Construction performance
blockbinary<8>    construction      1048577 per       0.0004636sec ->   2 Gops/sec
blockbinary<16>   construction      1048577 per       0.0006471sec ->   1 Gops/sec
blockbinary<32>   construction      1048577 per        0.000593sec ->   1 Gops/sec
blockbinary<64>   construction      1048577 per       0.0005512sec ->   1 Gops/sec
blockbinary<128>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockbinary<256>  construction      1048577 per               0sec ->   0  ops/sec
blockbinary<512>  construction      1048577 per       0.0076663sec -> 136 Mops/sec
blockbinary<1024> construction      1048577 per       0.0087083sec -> 120 Mops/sec

Block size performance on logical shift operators
blockbinary<8,uint8>     shifts      1048576 per               0sec ->   0  ops/sec
blockbinary<16,uint8>    shifts      1048576 per       0.0106529sec ->  98 Mops/sec
blockbinary<16,uint16>   shifts      1048576 per       0.0114639sec ->  91 Mops/sec
blockbinary<32,uint8>    shifts      1048576 per       0.0128313sec ->  81 Mops/sec
blockbinary<32,uint16>   shifts      1048576 per       0.0126253sec ->  83 Mops/sec
blockbinary<32,uint32>   shifts      1048576 per       0.0105466sec ->  99 Mops/sec
blockbinary<64,uint8>    shifts      1048576 per       0.0189186sec ->  55 Mops/sec
blockbinary<64,uint16>   shifts      1048576 per       0.0150479sec ->  69 Mops/sec
blockbinary<64,uint32>   shifts      1048576 per       0.0111554sec ->  93 Mops/sec
blockbinary<128,uint8>   shifts       524288 per         0.01392sec ->  37 Mops/sec
blockbinary<128,uint16>  shifts       524288 per       0.0094553sec ->  55 Mops/sec
blockbinary<128,uint32>  shifts       524288 per       0.0068041sec ->  77 Mops/sec
blockbinary<256,uint8>   shifts       262144 per       0.0169485sec ->  15 Mops/sec
blockbinary<256,uint16>  shifts       262144 per       0.0071399sec ->  36 Mops/sec
blockbinary<256,uint32>  shifts       262144 per       0.0044277sec ->  59 Mops/sec
blockbinary<512,uint8>   shifts       131072 per       0.0283206sec ->   4 Mops/sec
blockbinary<512,uint16>  shifts       131072 per       0.0132739sec ->   9 Mops/sec
blockbinary<512,uint32>  shifts       131072 per       0.0073879sec ->  17 Mops/sec
blockbinary<1024,uint8>  shifts        65536 per       0.0141784sec ->   4 Mops/sec
blockbinary<1024,uint16> shifts        65536 per        0.006632sec ->   9 Mops/sec
blockbinary<1024,uint32> shifts        65536 per       0.0036941sec ->  17 Mops/sec

ADDITION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      add       2097152 per       0.0016234sec ->   1 Gops/sec
blockbinary<8,uint8>      add       2097152 per       0.0010308sec ->   2 Gops/sec
blockbinary<16,uint8>     add       2097152 per       0.0110903sec -> 189 Mops/sec
blockbinary<16,uint16>    add       2097152 per       0.0013735sec ->   1 Gops/sec
blockbinary<32,uint8>     add       2097152 per       0.0160905sec -> 130 Mops/sec
blockbinary<32,uint16>    add       2097152 per       0.0133277sec -> 157 Mops/sec
blockbinary<32,uint32>    add       2097152 per       0.0012633sec ->   1 Gops/sec
blockbinary<64,uint8>     add       2097152 per       0.0251509sec ->  83 Mops/sec
blockbinary<64,uint16>    add       2097152 per       0.0160171sec -> 130 Mops/sec
blockbinary<64,uint32>    add       2097152 per       0.0109903sec -> 190 Mops/sec
blockbinary<128,uint8>    add       1048576 per       0.0219925sec ->  47 Mops/sec
blockbinary<128,uint16>   add       1048576 per        0.013093sec ->  80 Mops/sec
blockbinary<128,uint32>   add       1048576 per       0.0084677sec -> 123 Mops/sec
blockbinary<256,uint8>    add        524288 per       0.0160314sec ->  32 Mops/sec
blockbinary<256,uint16>   add        524288 per       0.0078161sec ->  67 Mops/sec
blockbinary<256,uint32>   add        524288 per       0.0047625sec -> 110 Mops/sec
blockbinary<512,uint8>    add        262144 per       0.0173674sec ->  15 Mops/sec
blockbinary<512,uint16>   add        262144 per       0.0082287sec ->  31 Mops/sec
blockbinary<512,uint32>   add        262144 per       0.0039234sec ->  66 Mops/sec
blockbinary<1024,uint8>   add        131072 per       0.0184005sec ->   7 Mops/sec
blockbinary<1024,uint16>  add        131072 per        0.008715sec ->  15 Mops/sec
blockbinary<1024,uint32>  add        131072 per       0.0043875sec ->  29 Mops/sec

MULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      mul        524288 per       0.0016812sec -> 311 Mops/sec
blockbinary<8,uint8>      mul        524288 per       0.0025846sec -> 202 Mops/sec
blockbinary<16,uint8>     mul        524288 per       0.0108979sec ->  48 Mops/sec
blockbinary<16,uint16>    mul        524288 per       0.0052016sec -> 100 Mops/sec
blockbinary<32,uint8>     mul        524288 per       0.0363337sec ->  14 Mops/sec
blockbinary<32,uint16>    mul        524288 per        0.020125sec ->  26 Mops/sec
blockbinary<32,uint32>    mul        524288 per       0.0092914sec ->  56 Mops/sec
blockbinary<64,uint8>     mul        524288 per        0.134975sec ->   3 Mops/sec
blockbinary<64,uint16>    mul        524288 per       0.0750144sec ->   6 Mops/sec
blockbinary<64,uint32>    mul        524288 per       0.0389709sec ->  13 Mops/sec
blockbinary<128,uint8>    mul        262144 per        0.434769sec -> 602 Kops/sec
blockbinary<128,uint16>   mul        262144 per        0.159382sec ->   1 Mops/sec
blockbinary<128,uint32>   mul        262144 per       0.0733774sec ->   3 Mops/sec
blockbinary<256,uint8>    mul         32768 per        0.322315sec -> 101 Kops/sec
blockbinary<256,uint16>   mul         65536 per        0.221012sec -> 296 Kops/sec
blockbinary<256,uint32>   mul        131072 per        0.146062sec -> 897 Kops/sec
blockbinary<512,uint8>    mul          1024 per       0.0419586sec ->  24 Kops/sec
blockbinary<512,uint16>   mul          2048 per       0.0415352sec ->  49 Kops/sec
blockbinary<512,uint32>   mul          4096 per       0.0264407sec -> 154 Kops/sec
blockbinary<1024,uint8>   mul           512 per       0.0669119sec ->   7 Kops/sec
blockbinary<1024,uint16>  mul          1024 per       0.0841949sec ->  12 Kops/sec
blockbinary<1024,uint32>  mul          2048 per        0.084731sec ->  24 Kops/sec

DIVISION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      div       1048576 per       0.0211397sec ->  49 Mops/sec
blockbinary<8,uint8>      div       1048576 per       0.0468243sec ->  22 Mops/sec
blockbinary<16,uint8>     div       1048576 per       0.0543239sec ->  19 Mops/sec
blockbinary<16,uint16>    div       1048576 per       0.0504223sec ->  20 Mops/sec
blockbinary<32,uint8>     div       1048576 per       0.0658134sec ->  15 Mops/sec
blockbinary<32,uint16>    div       1048576 per       0.0543883sec ->  19 Mops/sec
blockbinary<32,uint32>    div       1048576 per       0.0605002sec ->  17 Mops/sec
blockbinary<64,uint8>     div       1048576 per        0.125781sec ->   8 Mops/sec
blockbinary<64,uint16>    div       1048576 per       0.0871741sec ->  12 Mops/sec
blockbinary<64,uint32>    div       1048576 per       0.0703951sec ->  14 Mops/sec
blockbinary<128,uint8>    div        524288 per        0.143691sec ->   3 Mops/sec
blockbinary<128,uint16>   div        524288 per       0.0675138sec ->   7 Mops/sec
blockbinary<128,uint32>   div        524288 per        0.046778sec ->  11 Mops/sec
blockbinary<256,uint8>    div        262144 per        0.142969sec ->   1 Mops/sec
blockbinary<256,uint16>   div        262144 per       0.0773146sec ->   3 Mops/sec
blockbinary<256,uint32>   div        262144 per       0.0377292sec ->   6 Mops/sec
blockbinary<512,uint8>    div        131072 per        0.147371sec -> 889 Kops/sec
blockbinary<512,uint16>   div        131072 per       0.0795004sec ->   1 Mops/sec
blockbinary<512,uint32>   div        131072 per       0.0414952sec ->   3 Mops/sec
blockbinary<1024,uint8>   div         65536 per         0.15241sec -> 429 Kops/sec
blockbinary<1024,uint16>  div         65536 per       0.0779025sec -> 841 Kops/sec
blockbinary<1024,uint32>  div         65536 per       0.0400795sec ->   1 Mops/sec

REMAINDER: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      rem       1048576 per       0.0165224sec ->  63 Mops/sec
blockbinary<8,uint8>      rem       1048576 per       0.0250585sec ->  41 Mops/sec
blockbinary<16,uint8>     rem       1048576 per       0.0371575sec ->  28 Mops/sec
blockbinary<16,uint16>    rem       1048576 per       0.0279774sec ->  37 Mops/sec
blockbinary<32,uint8>     rem       1048576 per        0.043484sec ->  24 Mops/sec
blockbinary<32,uint16>    rem       1048576 per       0.0381043sec ->  27 Mops/sec
blockbinary<32,uint32>    rem       1048576 per        0.045502sec ->  23 Mops/sec
blockbinary<64,uint8>     rem       1048576 per       0.0680178sec ->  15 Mops/sec
blockbinary<64,uint16>    rem       1048576 per       0.0517253sec ->  20 Mops/sec
blockbinary<64,uint32>    rem       1048576 per        0.044923sec ->  23 Mops/sec
blockbinary<128,uint8>    rem        524288 per       0.0482109sec ->  10 Mops/sec
blockbinary<128,uint16>   rem        524288 per       0.0235673sec ->  22 Mops/sec
blockbinary<128,uint32>   rem        524288 per       0.0177471sec ->  29 Mops/sec
blockbinary<256,uint8>    rem        262144 per       0.0499646sec ->   5 Mops/sec
blockbinary<256,uint16>   rem        262144 per        0.028468sec ->   9 Mops/sec
blockbinary<256,uint32>   rem        262144 per       0.0125149sec ->  20 Mops/sec
blockbinary<512,uint8>    rem        131072 per       0.0541537sec ->   2 Mops/sec
blockbinary<512,uint16>   rem        131072 per       0.0292839sec ->   4 Mops/sec
blockbinary<512,uint32>   rem        131072 per       0.0138712sec ->   9 Mops/sec
blockbinary<1024,uint8>   rem         65536 per       0.0591468sec ->   1 Mops/sec
blockbinary<1024,uint16>  rem         65536 per       0.0286567sec ->   2 Mops/sec
blockbinary<1024,uint32>  rem         65536 per       0.0147311sec ->   4 Mops/sec
*/


/*
ETLO
Date run : 03/19/2022
Processor: AMD Ryzen 7 2700X Eight-Core Processor 3.70 GHz desktop 105W processor
Cache    : L1 96KB/core (768kB total), L2 512k/core (4.0MB total), L3 16.0MB
Memory   : 32GB
System   : 64-bit Windows 11 Pro, Version 21H2, x64-based processor, OS build 22000.556

blockbinary operator performance benchmarking

Logical shift operator performance
blockbinary<16>   shifts            1048576 per       0.0129976sec ->  80 Mops/sec
blockbinary<32>   shifts            1048576 per        0.017511sec ->  59 Mops/sec
blockbinary<64>   shifts            1048576 per       0.0270338sec ->  38 Mops/sec
blockbinary<128>  shifts             524288 per       0.0176927sec ->  29 Mops/sec
blockbinary<256>  shifts             262144 per       0.0190321sec ->  13 Mops/sec
blockbinary<512>  shifts             131072 per       0.0247181sec ->   5 Mops/sec
blockbinary<1024> shifts              65536 per       0.0180043sec ->   3 Mops/sec

Arithmetic operator performance
blockbinary<16>   add/subtract      2097152 per       0.0177148sec -> 118 Mops/sec
blockbinary<32>   add/subtract      2097152 per       0.0238561sec ->  87 Mops/sec
blockbinary<64>   add/subtract      2097152 per        0.034997sec ->  59 Mops/sec
blockbinary<128>  add/subtract      1048576 per       0.0250035sec ->  41 Mops/sec
blockbinary<256>  add/subtract       524288 per       0.0186836sec ->  28 Mops/sec
blockbinary<512>  add/subtract       262144 per       0.0179957sec ->  14 Mops/sec
blockbinary<1024> add/subtract       131072 per       0.0177477sec ->   7 Mops/sec
blockbinary<16>   multiplication    1048576 per        0.024258sec ->  43 Mops/sec
blockbinary<32>   multiplication     524288 per       0.0428356sec ->  12 Mops/sec
blockbinary<64>   multiplication     262144 per       0.0772769sec ->   3 Mops/sec
blockbinary<128>  multiplication      16384 per       0.0363821sec -> 450 Kops/sec
blockbinary<512>  multiplication       2048 per       0.0747159sec ->  27 Kops/sec
blockbinary<1024> multiplication       1024 per        0.153858sec ->   6 Kops/sec
blockbinary<16>   division           524288 per       0.0395495sec ->  13 Mops/sec
blockbinary<32>   division           524288 per       0.0431528sec ->  12 Mops/sec
blockbinary<64>   division           262144 per       0.0405781sec ->   6 Mops/sec
blockbinary<128>  division           131072 per       0.0491489sec ->   2 Mops/sec
blockbinary<512>  division            65536 per       0.0921349sec -> 711 Kops/sec
blockbinary<1024> division            32768 per       0.0901406sec -> 363 Kops/sec
blockbinary<16>   remainder          524288 per       0.0240982sec ->  21 Mops/sec
blockbinary<32>   remainder          524288 per       0.0294039sec ->  17 Mops/sec
blockbinary<64>   remainder          262144 per       0.0247215sec ->  10 Mops/sec
blockbinary<128>  remainder          131072 per       0.0172034sec ->   7 Mops/sec
blockbinary<512>  remainder           65536 per       0.0443009sec ->   1 Mops/sec
blockbinary<1024> remainder           32768 per       0.0372859sec -> 878 Kops/sec

Construction performance
blockbinary<8>    construction      1048577 per       0.0007161sec ->   1 Gops/sec
blockbinary<16>   construction      1048577 per       0.0007387sec ->   1 Gops/sec
blockbinary<32>   construction      1048577 per       0.0008148sec ->   1 Gops/sec
blockbinary<64>   construction      1048577 per       0.0006218sec ->   1 Gops/sec
blockbinary<128>  construction      1048577 per           2e-07sec ->   5 Tops/sec
blockbinary<256>  construction      1048577 per           2e-07sec ->   5 Tops/sec
blockbinary<512>  construction      1048577 per       0.0061047sec -> 171 Mops/sec
blockbinary<1024> construction      1048577 per       0.0244186sec ->  42 Mops/sec

Block size performance on logical shift operators
blockbinary<8,uint8>     shifts      1048576 per           2e-07sec ->   5 Tops/sec
blockbinary<16,uint8>    shifts      1048576 per       0.0133041sec ->  78 Mops/sec
blockbinary<16,uint16>   shifts      1048576 per        0.014597sec ->  71 Mops/sec
blockbinary<32,uint8>    shifts      1048576 per       0.0171373sec ->  61 Mops/sec
blockbinary<32,uint16>   shifts      1048576 per       0.0166364sec ->  63 Mops/sec
blockbinary<32,uint32>   shifts      1048576 per       0.0183361sec ->  57 Mops/sec
blockbinary<64,uint8>    shifts      1048576 per       0.0295841sec ->  35 Mops/sec
blockbinary<64,uint16>   shifts      1048576 per       0.0198625sec ->  52 Mops/sec
blockbinary<64,uint32>   shifts      1048576 per       0.0142957sec ->  73 Mops/sec
blockbinary<128,uint8>   shifts       524288 per       0.0219723sec ->  23 Mops/sec
blockbinary<128,uint16>  shifts       524288 per        0.011636sec ->  45 Mops/sec
blockbinary<128,uint32>  shifts       524288 per       0.0099171sec ->  52 Mops/sec
blockbinary<256,uint8>   shifts       262144 per       0.0259556sec ->  10 Mops/sec
blockbinary<256,uint16>  shifts       262144 per        0.009386sec ->  27 Mops/sec
blockbinary<256,uint32>  shifts       262144 per       0.0053249sec ->  49 Mops/sec
blockbinary<512,uint8>   shifts       131072 per       0.0353548sec ->   3 Mops/sec
blockbinary<512,uint16>  shifts       131072 per       0.0161226sec ->   8 Mops/sec
blockbinary<512,uint32>  shifts       131072 per       0.0083378sec ->  15 Mops/sec
blockbinary<1024,uint8>  shifts        65536 per         0.01787sec ->   3 Mops/sec
blockbinary<1024,uint16> shifts        65536 per       0.0080736sec ->   8 Mops/sec
blockbinary<1024,uint32> shifts        65536 per       0.0040697sec ->  16 Mops/sec

ADDITION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      add       2097152 per       0.0026306sec -> 797 Mops/sec
blockbinary<8,uint8>      add       2097152 per       0.0026321sec -> 796 Mops/sec
blockbinary<16,uint8>     add       2097152 per       0.0177226sec -> 118 Mops/sec
blockbinary<16,uint16>    add       2097152 per       0.0015842sec ->   1 Gops/sec
blockbinary<32,uint8>     add       2097152 per       0.0214241sec ->  97 Mops/sec
blockbinary<32,uint16>    add       2097152 per       0.0186483sec -> 112 Mops/sec
blockbinary<32,uint32>    add       2097152 per       0.0011682sec ->   1 Gops/sec
blockbinary<64,uint8>     add       2097152 per       0.0318072sec ->  65 Mops/sec
blockbinary<64,uint16>    add       2097152 per       0.0213641sec ->  98 Mops/sec
blockbinary<64,uint32>    add       2097152 per       0.0173347sec -> 120 Mops/sec
blockbinary<128,uint8>    add       1048576 per       0.0240713sec ->  43 Mops/sec
blockbinary<128,uint16>   add       1048576 per       0.0157647sec ->  66 Mops/sec
blockbinary<128,uint32>   add       1048576 per        0.011551sec ->  90 Mops/sec
blockbinary<256,uint8>    add        524288 per       0.0206505sec ->  25 Mops/sec
blockbinary<256,uint16>   add        524288 per       0.0090024sec ->  58 Mops/sec
blockbinary<256,uint32>   add        524288 per       0.0062628sec ->  83 Mops/sec
blockbinary<512,uint8>    add        262144 per       0.0193297sec ->  13 Mops/sec
blockbinary<512,uint16>   add        262144 per       0.0081938sec ->  31 Mops/sec
blockbinary<512,uint32>   add        262144 per       0.0041081sec ->  63 Mops/sec
blockbinary<1024,uint8>   add        131072 per       0.0183192sec ->   7 Mops/sec
blockbinary<1024,uint16>  add        131072 per        0.009946sec ->  13 Mops/sec
blockbinary<1024,uint32>  add        131072 per        0.004828sec ->  27 Mops/sec

MULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      mul        524288 per       0.0018977sec -> 276 Mops/sec
blockbinary<8,uint8>      mul        524288 per       0.0026256sec -> 199 Mops/sec
blockbinary<16,uint8>     mul        524288 per       0.0155587sec ->  33 Mops/sec
blockbinary<16,uint16>    mul        524288 per       0.0065909sec ->  79 Mops/sec
blockbinary<32,uint8>     mul        524288 per       0.0487189sec ->  10 Mops/sec
blockbinary<32,uint16>    mul        524288 per       0.0316798sec ->  16 Mops/sec
blockbinary<32,uint32>    mul        524288 per       0.0188914sec ->  27 Mops/sec
blockbinary<64,uint8>     mul        524288 per         0.17227sec ->   3 Mops/sec
blockbinary<64,uint16>    mul        524288 per       0.0814941sec ->   6 Mops/sec
blockbinary<64,uint32>    mul        524288 per       0.0412513sec ->  12 Mops/sec
blockbinary<128,uint8>    mul        262144 per        0.631923sec -> 414 Kops/sec
blockbinary<128,uint16>   mul        262144 per        0.192141sec ->   1 Mops/sec
blockbinary<128,uint32>   mul        262144 per       0.0699298sec ->   3 Mops/sec
blockbinary<256,uint8>    mul         32768 per        0.296732sec -> 110 Kops/sec
blockbinary<256,uint16>   mul         65536 per        0.317678sec -> 206 Kops/sec
blockbinary<256,uint32>   mul        131072 per        0.160059sec -> 818 Kops/sec
blockbinary<512,uint8>    mul          1024 per       0.0408546sec ->  25 Kops/sec
blockbinary<512,uint16>   mul          2048 per       0.0352479sec ->  58 Kops/sec
blockbinary<512,uint32>   mul          4096 per       0.0313558sec -> 130 Kops/sec
blockbinary<1024,uint8>   mul           512 per       0.0809457sec ->   6 Kops/sec
blockbinary<1024,uint16>  mul          1024 per       0.0820582sec ->  12 Kops/sec
blockbinary<1024,uint32>  mul          2048 per       0.0748518sec ->  27 Kops/sec

DIVISION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      div       1048576 per       0.0371902sec ->  28 Mops/sec
blockbinary<8,uint8>      div       1048576 per       0.0634376sec ->  16 Mops/sec
blockbinary<16,uint8>     div       1048576 per       0.0733217sec ->  14 Mops/sec
blockbinary<16,uint16>    div       1048576 per       0.0657642sec ->  15 Mops/sec
blockbinary<32,uint8>     div       1048576 per       0.0964202sec ->  10 Mops/sec
blockbinary<32,uint16>    div       1048576 per       0.0789437sec ->  13 Mops/sec
blockbinary<32,uint32>    div       1048576 per       0.0833805sec ->  12 Mops/sec
blockbinary<64,uint8>     div       1048576 per        0.184213sec ->   5 Mops/sec
blockbinary<64,uint16>    div       1048576 per        0.134421sec ->   7 Mops/sec
blockbinary<64,uint32>    div       1048576 per       0.0914573sec ->  11 Mops/sec
blockbinary<128,uint8>    div        524288 per        0.192709sec ->   2 Mops/sec
blockbinary<128,uint16>   div        524288 per       0.0914627sec ->   5 Mops/sec
blockbinary<128,uint32>   div        524288 per       0.0682722sec ->   7 Mops/sec
blockbinary<256,uint8>    div        262144 per        0.189627sec ->   1 Mops/sec
blockbinary<256,uint16>   div        262144 per           0.106sec ->   2 Mops/sec
blockbinary<256,uint32>   div        262144 per       0.0520821sec ->   5 Mops/sec
blockbinary<512,uint8>    div        131072 per        0.198784sec -> 659 Kops/sec
blockbinary<512,uint16>   div        131072 per         0.10001sec ->   1 Mops/sec
blockbinary<512,uint32>   div        131072 per       0.0553885sec ->   2 Mops/sec
blockbinary<1024,uint8>   div         65536 per        0.185807sec -> 352 Kops/sec
blockbinary<1024,uint16>  div         65536 per        0.105308sec -> 622 Kops/sec
blockbinary<1024,uint32>  div         65536 per       0.0561244sec ->   1 Mops/sec

REMAINDER: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      rem       1048576 per       0.0168175sec ->  62 Mops/sec
blockbinary<8,uint8>      rem       1048576 per       0.0389062sec ->  26 Mops/sec
blockbinary<16,uint8>     rem       1048576 per       0.0474354sec ->  22 Mops/sec
blockbinary<16,uint16>    rem       1048576 per       0.0323672sec ->  32 Mops/sec
blockbinary<32,uint8>     rem       1048576 per        0.058221sec ->  18 Mops/sec
blockbinary<32,uint16>    rem       1048576 per       0.0504034sec ->  20 Mops/sec
blockbinary<32,uint32>    rem       1048576 per       0.0582756sec ->  17 Mops/sec
blockbinary<64,uint8>     rem       1048576 per        0.121894sec ->   8 Mops/sec
blockbinary<64,uint16>    rem       1048576 per       0.0647446sec ->  16 Mops/sec
blockbinary<64,uint32>    rem       1048576 per       0.0641945sec ->  16 Mops/sec
blockbinary<128,uint8>    rem        524288 per        0.064757sec ->   8 Mops/sec
blockbinary<128,uint16>   rem        524288 per       0.0307583sec ->  17 Mops/sec
blockbinary<128,uint32>   rem        524288 per       0.0188096sec ->  27 Mops/sec
blockbinary<256,uint8>    rem        262144 per        0.078893sec ->   3 Mops/sec
blockbinary<256,uint16>   rem        262144 per       0.0356271sec ->   7 Mops/sec
blockbinary<256,uint32>   rem        262144 per        0.016986sec ->  15 Mops/sec
blockbinary<512,uint8>    rem        131072 per       0.0898092sec ->   1 Mops/sec
blockbinary<512,uint16>   rem        131072 per       0.0424889sec ->   3 Mops/sec
blockbinary<512,uint32>   rem        131072 per       0.0169454sec ->   7 Mops/sec
blockbinary<1024,uint8>   rem         65536 per       0.0723782sec -> 905 Kops/sec
blockbinary<1024,uint16>  rem         65536 per       0.0499046sec ->   1 Mops/sec
blockbinary<1024,uint32>  rem         65536 per       0.0196345sec ->   3 Mops/sec

changing operator+=() to use pointers and an arithmetic carry: about a 10% performance gain
blockbinary<16>   add/subtract      2097152 per       0.0172887sec -> 121 Mops/sec
blockbinary<32>   add/subtract      2097152 per       0.0198335sec -> 105 Mops/sec
blockbinary<64>   add/subtract      2097152 per       0.0258789sec ->  81 Mops/sec
blockbinary<128>  add/subtract      1048576 per        0.019928sec ->  52 Mops/sec
blockbinary<256>  add/subtract       524288 per       0.0164414sec ->  31 Mops/sec
blockbinary<512>  add/subtract       262144 per       0.0141651sec ->  18 Mops/sec
blockbinary<1024> add/subtract       131072 per       0.0149449sec ->   8 Mops/sec

blockbinary<4,uint8>      add       2097152 per       0.0026287sec -> 797 Mops/sec
blockbinary<8,uint8>      add       2097152 per       0.0026241sec -> 799 Mops/sec
blockbinary<16,uint8>     add       2097152 per       0.0195565sec -> 107 Mops/sec
blockbinary<16,uint16>    add       2097152 per       0.0015823sec ->   1 Gops/sec
blockbinary<32,uint8>     add       2097152 per       0.0193255sec -> 108 Mops/sec
blockbinary<32,uint16>    add       2097152 per       0.0173402sec -> 120 Mops/sec
blockbinary<32,uint32>    add       2097152 per       0.0015711sec ->   1 Gops/sec
blockbinary<64,uint8>     add       2097152 per       0.0257522sec ->  81 Mops/sec  << from 65Mops
blockbinary<64,uint16>    add       2097152 per       0.0210924sec ->  99 Mops/sec
blockbinary<64,uint32>    add       2097152 per       0.0172938sec -> 121 Mops/sec
blockbinary<128,uint8>    add       1048576 per       0.0197569sec ->  53 Mops/sec
blockbinary<128,uint16>   add       1048576 per       0.0127447sec ->  82 Mops/sec
blockbinary<128,uint32>   add       1048576 per        0.010513sec ->  99 Mops/sec  << from 90Mops
blockbinary<256,uint8>    add        524288 per       0.0185148sec ->  28 Mops/sec
blockbinary<256,uint16>   add        524288 per       0.0104053sec ->  50 Mops/sec
blockbinary<256,uint32>   add        524288 per       0.0054061sec ->  96 Mops/sec  << from 83Mops
blockbinary<512,uint8>    add        262144 per       0.0141602sec ->  18 Mops/sec
blockbinary<512,uint16>   add        262144 per       0.0079915sec ->  32 Mops/sec
blockbinary<512,uint32>   add        262144 per       0.0037765sec ->  69 Mops/sec  << from 63Mops
blockbinary<1024,uint8>   add        131072 per       0.0146829sec ->   8 Mops/sec
blockbinary<1024,uint16>  add        131072 per       0.0077309sec ->  16 Mops/sec
blockbinary<1024,uint32>  add        131072 per       0.0038901sec ->  33 Mops/sec  << from 27Mops
*/

/*
ETLO
Date run : 03/24/2022
Processor: AMD Ryzen 7 2700X Eight-Core Processor 3.70 GHz desktop 105W processor
Cache    : L1 96KB/core (768kB total), L2 512k/core (4.0MB total), L3 16.0MB
Memory   : 32GB
System   : 64-bit Windows 11 Pro, Version 21H2, x64-based processor, OS build 22000.556

blockbinary operator performance benchmarking

Logical shift operator performance
blockbinary<16>   shifts            1048576 per       0.0118465sec ->  88 Mops/sec
blockbinary<32>   shifts            1048576 per       0.0148215sec ->  70 Mops/sec
blockbinary<64>   shifts            1048576 per        0.026277sec ->  39 Mops/sec
blockbinary<128>  shifts             524288 per       0.0177195sec ->  29 Mops/sec
blockbinary<256>  shifts             262144 per        0.021125sec ->  12 Mops/sec
blockbinary<512>  shifts             131072 per       0.0185274sec ->   7 Mops/sec
blockbinary<1024> shifts              65536 per       0.0189151sec ->   3 Mops/sec

Arithmetic operator performance
blockbinary<16>   add/subtract      2097152 per        0.017533sec -> 119 Mops/sec
blockbinary<32>   add/subtract      2097152 per       0.0187578sec -> 111 Mops/sec
blockbinary<64>   add/subtract      2097152 per       0.0249671sec ->  83 Mops/sec
blockbinary<128>  add/subtract      1048576 per       0.0197862sec ->  52 Mops/sec
blockbinary<256>  add/subtract       524288 per       0.0159773sec ->  32 Mops/sec
blockbinary<512>  add/subtract       262144 per       0.0147155sec ->  17 Mops/sec
blockbinary<1024> add/subtract       131072 per       0.0146667sec ->   8 Mops/sec
blockbinary<16>   multiplication    1048576 per       0.0263803sec ->  39 Mops/sec
blockbinary<32>   multiplication     524288 per       0.0170241sec ->  30 Mops/sec
blockbinary<64>   multiplication     262144 per        0.025511sec ->  10 Mops/sec
blockbinary<128>  multiplication      16384 per       0.0087244sec ->   1 Mops/sec
blockbinary<512>  multiplication       2048 per       0.0145292sec -> 140 Kops/sec
blockbinary<1024> multiplication       1024 per       0.0283936sec ->  36 Kops/sec
blockbinary<16>   division           524288 per       0.0653847sec ->   8 Mops/sec
blockbinary<32>   division           524288 per       0.0755993sec ->   6 Mops/sec
blockbinary<64>   division           262144 per       0.0515325sec ->   5 Mops/sec
blockbinary<128>  division           131072 per       0.0474441sec ->   2 Mops/sec
blockbinary<512>  division            65536 per       0.0869284sec -> 753 Kops/sec
blockbinary<1024> division            32768 per       0.0779117sec -> 420 Kops/sec
blockbinary<16>   remainder          524288 per        0.041208sec ->  12 Mops/sec
blockbinary<32>   remainder          524288 per       0.0454182sec ->  11 Mops/sec
blockbinary<64>   remainder          262144 per       0.0311969sec ->   8 Mops/sec
blockbinary<128>  remainder          131072 per       0.0157432sec ->   8 Mops/sec
blockbinary<512>  remainder           65536 per       0.0372441sec ->   1 Mops/sec
blockbinary<1024> remainder           32768 per       0.0317347sec ->   1 Mops/sec

Construction performance
blockbinary<8>    construction      1048577 per       0.0006249sec ->   1 Gops/sec
blockbinary<16>   construction      1048577 per       0.0007346sec ->   1 Gops/sec
blockbinary<32>   construction      1048577 per       0.0007782sec ->   1 Gops/sec
blockbinary<64>   construction      1048577 per       0.0006151sec ->   1 Gops/sec
blockbinary<128>  construction      1048577 per               0sec ->   0  ops/sec
blockbinary<256>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockbinary<512>  construction      1048577 per       0.0063895sec -> 164 Mops/sec
blockbinary<1024> construction      1048577 per       0.0236022sec ->  44 Mops/sec

Block size performance on logical shift operators
blockbinary<8,uint8>     shifts      1048576 per           1e-07sec ->  10 Tops/sec
blockbinary<16,uint8>    shifts      1048576 per        0.011668sec ->  89 Mops/sec
blockbinary<16,uint16>   shifts      1048576 per       0.0154543sec ->  67 Mops/sec
blockbinary<32,uint8>    shifts      1048576 per        0.015587sec ->  67 Mops/sec
blockbinary<32,uint16>   shifts      1048576 per       0.0151032sec ->  69 Mops/sec
blockbinary<32,uint32>   shifts      1048576 per        0.012905sec ->  81 Mops/sec
blockbinary<64,uint8>    shifts      1048576 per       0.0284352sec ->  36 Mops/sec
blockbinary<64,uint16>   shifts      1048576 per       0.0192402sec ->  54 Mops/sec
blockbinary<64,uint32>   shifts      1048576 per       0.0136261sec ->  76 Mops/sec
blockbinary<128,uint8>   shifts       524288 per       0.0241527sec ->  21 Mops/sec
blockbinary<128,uint16>  shifts       524288 per       0.0110808sec ->  47 Mops/sec
blockbinary<128,uint32>  shifts       524288 per       0.0079477sec ->  65 Mops/sec
blockbinary<256,uint8>   shifts       262144 per       0.0229113sec ->  11 Mops/sec
blockbinary<256,uint16>  shifts       262144 per        0.012912sec ->  20 Mops/sec
blockbinary<256,uint32>  shifts       262144 per       0.0088701sec ->  29 Mops/sec
blockbinary<512,uint8>   shifts       131072 per       0.0381152sec ->   3 Mops/sec
blockbinary<512,uint16>  shifts       131072 per       0.0183083sec ->   7 Mops/sec
blockbinary<512,uint32>  shifts       131072 per       0.0080144sec ->  16 Mops/sec
blockbinary<1024,uint8>  shifts        65536 per       0.0193393sec ->   3 Mops/sec
blockbinary<1024,uint16> shifts        65536 per       0.0093148sec ->   7 Mops/sec
blockbinary<1024,uint32> shifts        65536 per       0.0042051sec ->  15 Mops/sec

ADDITION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      add       2097152 per       0.0025977sec -> 807 Mops/sec
blockbinary<8,uint8>      add       2097152 per       0.0026039sec -> 805 Mops/sec
blockbinary<16,uint8>     add       2097152 per       0.0173398sec -> 120 Mops/sec
blockbinary<16,uint16>    add       2097152 per       0.0015822sec ->   1 Gops/sec
blockbinary<32,uint8>     add       2097152 per       0.0188803sec -> 111 Mops/sec
blockbinary<32,uint16>    add       2097152 per       0.0176656sec -> 118 Mops/sec
blockbinary<32,uint32>    add       2097152 per       0.0015556sec ->   1 Gops/sec
blockbinary<64,uint8>     add       2097152 per       0.0252756sec ->  82 Mops/sec
blockbinary<64,uint16>    add       2097152 per       0.0195738sec -> 107 Mops/sec
blockbinary<64,uint32>    add       2097152 per       0.0172874sec -> 121 Mops/sec
blockbinary<128,uint8>    add       1048576 per       0.0197369sec ->  53 Mops/sec
blockbinary<128,uint16>   add       1048576 per       0.0129376sec ->  81 Mops/sec
blockbinary<128,uint32>   add       1048576 per       0.0104244sec -> 100 Mops/sec
blockbinary<256,uint8>    add        524288 per       0.0216207sec ->  24 Mops/sec
blockbinary<256,uint16>   add        524288 per       0.0081355sec ->  64 Mops/sec
blockbinary<256,uint32>   add        524288 per       0.0053925sec ->  97 Mops/sec
blockbinary<512,uint8>    add        262144 per       0.0139172sec ->  18 Mops/sec
blockbinary<512,uint16>   add        262144 per        0.008018sec ->  32 Mops/sec
blockbinary<512,uint32>   add        262144 per       0.0037678sec ->  69 Mops/sec
blockbinary<1024,uint8>   add        131072 per       0.0144597sec ->   9 Mops/sec
blockbinary<1024,uint16>  add        131072 per        0.007674sec ->  17 Mops/sec
blockbinary<1024,uint32>  add        131072 per       0.0039097sec ->  33 Mops/sec

MULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      mul        524288 per       0.0019124sec -> 274 Mops/sec
blockbinary<8,uint8>      mul        524288 per       0.0076672sec ->  68 Mops/sec   <-- !!! was 199 Mops/sec
blockbinary<16,uint8>     mul        524288 per        0.013102sec ->  40 Mops/sec
blockbinary<16,uint16>    mul        524288 per       0.0075564sec ->  69 Mops/sec   <-- !!! was  79 Mops/sec
blockbinary<32,uint8>     mul        524288 per       0.0169468sec ->  30 Mops/sec
blockbinary<32,uint16>    mul        524288 per       0.0124035sec ->  42 Mops/sec
blockbinary<32,uint32>    mul        524288 per       0.0075307sec ->  69 Mops/sec
blockbinary<64,uint8>     mul        524288 per       0.0502273sec ->  10 Mops/sec
blockbinary<64,uint16>    mul        524288 per       0.0176967sec ->  29 Mops/sec
blockbinary<64,uint32>    mul        524288 per       0.0133068sec ->  39 Mops/sec
blockbinary<128,uint8>    mul        262144 per        0.138187sec ->   1 Mops/sec
blockbinary<128,uint16>   mul        262144 per       0.0276339sec ->   9 Mops/sec
blockbinary<128,uint32>   mul        262144 per       0.0103772sec ->  25 Mops/sec   <-- from   3 Mops/sec  8x better
blockbinary<256,uint8>    mul         32768 per       0.0709573sec -> 461 Kops/sec   <-- from 110 Kops/sec  4x
blockbinary<256,uint16>   mul         65536 per       0.0369608sec ->   1 Mops/sec   <-- from 206 Kops/sec  5x
blockbinary<256,uint32>   mul        131072 per       0.0123171sec ->  10 Mops/sec   <-- from 818 Kops/sec 12x better
blockbinary<512,uint8>    mul          1024 per       0.0070006sec -> 146 Kops/sec
blockbinary<512,uint16>   mul          2048 per        0.004195sec -> 488 Kops/sec
blockbinary<512,uint32>   mul          4096 per       0.0022381sec ->   1 Mops/sec
blockbinary<1024,uint8>   mul           512 per       0.0141246sec ->  36 Kops/sec
blockbinary<1024,uint16>  mul          1024 per       0.0070711sec -> 144 Kops/sec
blockbinary<1024,uint32>  mul          2048 per       0.0043079sec -> 475 Kops/sec

DIVISION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      div       1048576 per       0.0249495sec ->  42 Mops/sec
blockbinary<8,uint8>      div       1048576 per        0.127015sec ->   8 Mops/sec
blockbinary<16,uint8>     div       1048576 per         0.15167sec ->   6 Mops/sec
blockbinary<16,uint16>    div       1048576 per        0.134709sec ->   7 Mops/sec
blockbinary<32,uint8>     div       1048576 per        0.156926sec ->   6 Mops/sec
blockbinary<32,uint16>    div       1048576 per        0.140082sec ->   7 Mops/sec
blockbinary<32,uint32>    div       1048576 per        0.132171sec ->   7 Mops/sec
blockbinary<64,uint8>     div       1048576 per        0.209436sec ->   5 Mops/sec
blockbinary<64,uint16>    div       1048576 per        0.169668sec ->   6 Mops/sec
blockbinary<64,uint32>    div       1048576 per        0.167902sec ->   6 Mops/sec
blockbinary<128,uint8>    div        524288 per        0.188578sec ->   2 Mops/sec
blockbinary<128,uint16>   div        524288 per        0.118752sec ->   4 Mops/sec
blockbinary<128,uint32>   div        524288 per       0.0943648sec ->   5 Mops/sec
blockbinary<256,uint8>    div        262144 per        0.169591sec ->   1 Mops/sec
blockbinary<256,uint16>   div        262144 per        0.107404sec ->   2 Mops/sec
blockbinary<256,uint32>   div        262144 per       0.0610653sec ->   4 Mops/sec
blockbinary<512,uint8>    div        131072 per        0.184335sec -> 711 Kops/sec
blockbinary<512,uint16>   div        131072 per        0.110745sec ->   1 Mops/sec
blockbinary<512,uint32>   div        131072 per       0.0567486sec ->   2 Mops/sec
blockbinary<1024,uint8>   div         65536 per        0.157231sec -> 416 Kops/sec
blockbinary<1024,uint16>  div         65536 per        0.102118sec -> 641 Kops/sec
blockbinary<1024,uint32>  div         65536 per        0.052607sec ->   1 Mops/sec

REMAINDER: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      rem       1048576 per       0.0179058sec ->  58 Mops/sec
blockbinary<8,uint8>      rem       1048576 per       0.0620709sec ->  16 Mops/sec
blockbinary<16,uint8>     rem       1048576 per       0.0796432sec ->  13 Mops/sec
blockbinary<16,uint16>    rem       1048576 per       0.0657021sec ->  15 Mops/sec
blockbinary<32,uint8>     rem       1048576 per       0.0860172sec ->  12 Mops/sec
blockbinary<32,uint16>    rem       1048576 per       0.0774769sec ->  13 Mops/sec
blockbinary<32,uint32>    rem       1048576 per       0.0752144sec ->  13 Mops/sec
blockbinary<64,uint8>     rem       1048576 per        0.130109sec ->   8 Mops/sec
blockbinary<64,uint16>    rem       1048576 per       0.0962124sec ->  10 Mops/sec
blockbinary<64,uint32>    rem       1048576 per        0.100875sec ->  10 Mops/sec
blockbinary<128,uint8>    rem        524288 per       0.0621359sec ->   8 Mops/sec
blockbinary<128,uint16>   rem        524288 per       0.0368869sec ->  14 Mops/sec
blockbinary<128,uint32>   rem        524288 per       0.0265256sec ->  19 Mops/sec
blockbinary<256,uint8>    rem        262144 per       0.0687927sec ->   3 Mops/sec
blockbinary<256,uint16>   rem        262144 per       0.0408501sec ->   6 Mops/sec
blockbinary<256,uint32>   rem        262144 per       0.0187717sec ->  13 Mops/sec
blockbinary<512,uint8>    rem        131072 per       0.0725128sec ->   1 Mops/sec
blockbinary<512,uint16>   rem        131072 per       0.0408696sec ->   3 Mops/sec
blockbinary<512,uint32>   rem        131072 per       0.0191569sec ->   6 Mops/sec
blockbinary<1024,uint8>   rem         65536 per       0.0657663sec -> 996 Kops/sec
blockbinary<1024,uint16>  rem         65536 per       0.0395263sec ->   1 Mops/sec
blockbinary<1024,uint32>  rem         65536 per       0.0169843sec ->   3 Mops/sec


// performance improvement of special casing the fast multiply for single block representations:
MULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      mul        524288 per       0.0006819sec -> 768 Mops/sec
blockbinary<8,uint8>      mul        524288 per       0.0006634sec -> 790 Mops/sec   <-- was 68 Mops/sec
blockbinary<16,uint8>     mul        524288 per       0.0144837sec ->  36 Mops/sec
blockbinary<16,uint16>    mul        524288 per        0.000657sec -> 798 Mops/sec   <-- was 69 Mops/sec
blockbinary<32,uint8>     mul        524288 per       0.0170343sec ->  30 Mops/sec
blockbinary<32,uint16>    mul        524288 per       0.0125508sec ->  41 Mops/sec
blockbinary<32,uint32>    mul        524288 per       0.0004003sec ->   1 Gops/sec   <-- was 69 Mops/sec
blockbinary<64,uint8>     mul        524288 per       0.0567939sec ->   9 Mops/sec
blockbinary<64,uint16>    mul        524288 per         0.01865sec ->  28 Mops/sec
blockbinary<64,uint32>    mul        524288 per       0.0149327sec ->  35 Mops/sec
blockbinary<64,uint64>    mul        524288 per       0.0003911sec ->   1 Gops/sec
*/

/*
* ETLO
Date run : 03/24/2022
Processor: AMD Ryzen 7 2700X Eight-Core Processor 3.70 GHz desktop 105W processor
Cache    : L1 96KB/core (768kB total), L2 512k/core (4.0MB total), L3 16.0MB
Memory   : 32GB
System   : 64-bit Windows 11 Pro, Version 21H2, x64-based processor, OS build 22000.556

Added single block constexpr to native / and % instructions

blockbinary operator performance benchmarking

Logical shift operator performance
blockbinary<16>   shifts            1048576 per       0.0116823sec ->  89 Mops/sec
blockbinary<32>   shifts            1048576 per        0.014749sec ->  71 Mops/sec
blockbinary<64>   shifts            1048576 per       0.0251312sec ->  41 Mops/sec
blockbinary<128>  shifts             524288 per       0.0175844sec ->  29 Mops/sec
blockbinary<256>  shifts             262144 per        0.026816sec ->   9 Mops/sec
blockbinary<512>  shifts             131072 per       0.0183155sec ->   7 Mops/sec
blockbinary<1024> shifts              65536 per       0.0189369sec ->   3 Mops/sec

Arithmetic operator performance
blockbinary<16>   add/subtract      2097152 per       0.0172404sec -> 121 Mops/sec
blockbinary<32>   add/subtract      2097152 per        0.018812sec -> 111 Mops/sec
blockbinary<64>   add/subtract      2097152 per       0.0247139sec ->  84 Mops/sec
blockbinary<128>  add/subtract      1048576 per       0.0189751sec ->  55 Mops/sec
blockbinary<256>  add/subtract       524288 per        0.016788sec ->  31 Mops/sec
blockbinary<512>  add/subtract       262144 per       0.0145523sec ->  18 Mops/sec
blockbinary<1024> add/subtract       131072 per       0.0139069sec ->   9 Mops/sec
blockbinary<16>   multiplication    1048576 per       0.0252725sec ->  41 Mops/sec
blockbinary<32>   multiplication     524288 per       0.0164069sec ->  31 Mops/sec
blockbinary<64>   multiplication     262144 per       0.0230312sec ->  11 Mops/sec
blockbinary<128>  multiplication      16384 per       0.0082207sec ->   1 Mops/sec
blockbinary<512>  multiplication       2048 per        0.014316sec -> 143 Kops/sec
blockbinary<1024> multiplication       1024 per       0.0327912sec ->  31 Kops/sec
blockbinary<16>   division           524288 per       0.0685455sec ->   7 Mops/sec
blockbinary<32>   division           524288 per       0.0754646sec ->   6 Mops/sec
blockbinary<64>   division           262144 per       0.0571351sec ->   4 Mops/sec
blockbinary<128>  division           131072 per       0.0534136sec ->   2 Mops/sec
blockbinary<512>  division            65536 per        0.102156sec -> 641 Kops/sec
blockbinary<1024> division            32768 per       0.0792319sec -> 413 Kops/sec
blockbinary<16>   remainder          524288 per       0.0190876sec ->  27 Mops/sec
blockbinary<32>   remainder          524288 per       0.0236889sec ->  22 Mops/sec
blockbinary<64>   remainder          262144 per       0.0163022sec ->  16 Mops/sec
blockbinary<128>  remainder          131072 per       0.0148056sec ->   8 Mops/sec
blockbinary<512>  remainder           65536 per        0.023518sec ->   2 Mops/sec
blockbinary<1024> remainder           32768 per       0.0255662sec ->   1 Mops/sec

Construction performance
blockbinary<8>    construction      1048577 per       0.0006327sec ->   1 Gops/sec
blockbinary<16>   construction      1048577 per       0.0007478sec ->   1 Gops/sec
blockbinary<32>   construction      1048577 per       0.0006816sec ->   1 Gops/sec
blockbinary<64>   construction      1048577 per       0.0006779sec ->   1 Gops/sec
blockbinary<128>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockbinary<256>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockbinary<512>  construction      1048577 per       0.0095108sec -> 110 Mops/sec
blockbinary<1024> construction      1048577 per       0.0118007sec ->  88 Mops/sec

Block size performance on logical shift operators
blockbinary<8,uint8>     shifts      1048576 per           2e-07sec ->   5 Tops/sec
blockbinary<16,uint8>    shifts      1048576 per       0.0125212sec ->  83 Mops/sec
blockbinary<16,uint16>   shifts      1048576 per       0.0143297sec ->  73 Mops/sec
blockbinary<32,uint8>    shifts      1048576 per       0.0147135sec ->  71 Mops/sec
blockbinary<32,uint16>   shifts      1048576 per       0.0155164sec ->  67 Mops/sec
blockbinary<32,uint32>   shifts      1048576 per       0.0132875sec ->  78 Mops/sec
blockbinary<64,uint8>    shifts      1048576 per       0.0265977sec ->  39 Mops/sec
blockbinary<64,uint16>   shifts      1048576 per       0.0185746sec ->  56 Mops/sec
blockbinary<64,uint32>   shifts      1048576 per       0.0138005sec ->  75 Mops/sec
blockbinary<128,uint8>   shifts       524288 per       0.0177454sec ->  29 Mops/sec
blockbinary<128,uint16>  shifts       524288 per        0.011189sec ->  46 Mops/sec
blockbinary<128,uint32>  shifts       524288 per       0.0077678sec ->  67 Mops/sec
blockbinary<256,uint8>   shifts       262144 per       0.0224243sec ->  11 Mops/sec
blockbinary<256,uint16>  shifts       262144 per        0.009734sec ->  26 Mops/sec
blockbinary<256,uint32>  shifts       262144 per       0.0045312sec ->  57 Mops/sec
blockbinary<512,uint8>   shifts       131072 per        0.037488sec ->   3 Mops/sec
blockbinary<512,uint16>  shifts       131072 per       0.0174161sec ->   7 Mops/sec
blockbinary<512,uint32>  shifts       131072 per       0.0083762sec ->  15 Mops/sec
blockbinary<1024,uint8>  shifts        65536 per       0.0187959sec ->   3 Mops/sec
blockbinary<1024,uint16> shifts        65536 per       0.0078822sec ->   8 Mops/sec
blockbinary<1024,uint32> shifts        65536 per       0.0043133sec ->  15 Mops/sec

ADDITION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      add       2097152 per       0.0011941sec ->   1 Gops/sec
blockbinary<8,uint8>      add       2097152 per       0.0012076sec ->   1 Gops/sec
blockbinary<16,uint8>     add       2097152 per       0.0175809sec -> 119 Mops/sec
blockbinary<16,uint16>    add       2097152 per       0.0012437sec ->   1 Gops/sec
blockbinary<32,uint8>     add       2097152 per       0.0188236sec -> 111 Mops/sec
blockbinary<32,uint16>    add       2097152 per       0.0172814sec -> 121 Mops/sec
blockbinary<32,uint32>    add       2097152 per       0.0012327sec ->   1 Gops/sec
blockbinary<64,uint8>     add       2097152 per       0.0255468sec ->  82 Mops/sec
blockbinary<64,uint16>    add       2097152 per        0.018265sec -> 114 Mops/sec
blockbinary<64,uint32>    add       2097152 per       0.0173367sec -> 120 Mops/sec
blockbinary<128,uint8>    add       1048576 per       0.0194243sec ->  53 Mops/sec
blockbinary<128,uint16>   add       1048576 per       0.0127087sec ->  82 Mops/sec
blockbinary<128,uint32>   add       1048576 per       0.0104257sec -> 100 Mops/sec
blockbinary<256,uint8>    add        524288 per       0.0223988sec ->  23 Mops/sec
blockbinary<256,uint16>   add        524288 per       0.0073492sec ->  71 Mops/sec
blockbinary<256,uint32>   add        524288 per       0.0055375sec ->  94 Mops/sec
blockbinary<512,uint8>    add        262144 per       0.0141705sec ->  18 Mops/sec
blockbinary<512,uint16>   add        262144 per       0.0078309sec ->  33 Mops/sec
blockbinary<512,uint32>   add        262144 per       0.0047273sec ->  55 Mops/sec
blockbinary<1024,uint8>   add        131072 per       0.0146398sec ->   8 Mops/sec
blockbinary<1024,uint16>  add        131072 per       0.0077489sec ->  16 Mops/sec
blockbinary<1024,uint32>  add        131072 per       0.0047086sec ->  27 Mops/sec

MULTIPLICATION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      mul        524288 per       0.0007299sec -> 718 Mops/sec
blockbinary<8,uint8>      mul        524288 per       0.0006612sec -> 792 Mops/sec
blockbinary<16,uint8>     mul        524288 per       0.0131578sec ->  39 Mops/sec
blockbinary<16,uint16>    mul        524288 per       0.0006513sec -> 804 Mops/sec
blockbinary<32,uint8>     mul        524288 per       0.0171811sec ->  30 Mops/sec
blockbinary<32,uint16>    mul        524288 per        0.012801sec ->  40 Mops/sec
blockbinary<32,uint32>    mul        524288 per       0.0003912sec ->   1 Gops/sec
blockbinary<64,uint8>     mul        524288 per       0.0474884sec ->  11 Mops/sec
blockbinary<64,uint16>    mul        524288 per       0.0179347sec ->  29 Mops/sec
blockbinary<64,uint32>    mul        524288 per       0.0126257sec ->  41 Mops/sec
blockbinary<64,uint64>    mul        524288 per       0.0003965sec ->   1 Gops/sec
blockbinary<128,uint8>    mul        262144 per        0.138394sec ->   1 Mops/sec
blockbinary<128,uint16>   mul        262144 per       0.0249573sec ->  10 Mops/sec
blockbinary<128,uint32>   mul        262144 per       0.0096002sec ->  27 Mops/sec
blockbinary<256,uint8>    mul         32768 per       0.0698426sec -> 469 Kops/sec
blockbinary<256,uint16>   mul         65536 per        0.035309sec ->   1 Mops/sec
blockbinary<256,uint32>   mul        131072 per       0.0126634sec ->  10 Mops/sec
blockbinary<512,uint8>    mul          1024 per       0.0074712sec -> 137 Kops/sec
blockbinary<512,uint16>   mul          2048 per       0.0053826sec -> 380 Kops/sec
blockbinary<512,uint32>   mul          4096 per       0.0021807sec ->   1 Mops/sec
blockbinary<1024,uint8>   mul           512 per       0.0142634sec ->  35 Kops/sec
blockbinary<1024,uint16>  mul          1024 per       0.0072971sec -> 140 Kops/sec
blockbinary<1024,uint32>  mul          2048 per       0.0042326sec -> 483 Kops/sec

DIVISION: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      div       1048576 per       0.0256839sec ->  40 Mops/sec
blockbinary<8,uint8>      div       1048576 per       0.0039011sec -> 268 Mops/sec
blockbinary<16,uint8>     div       1048576 per        0.135858sec ->   7 Mops/sec
blockbinary<16,uint16>    div       1048576 per       0.0037758sec -> 277 Mops/sec
blockbinary<32,uint8>     div       1048576 per        0.158871sec ->   6 Mops/sec
blockbinary<32,uint16>    div       1048576 per        0.137241sec ->   7 Mops/sec
blockbinary<32,uint32>    div       1048576 per       0.0038577sec -> 271 Mops/sec
blockbinary<64,uint8>     div       1048576 per        0.219879sec ->   4 Mops/sec
blockbinary<64,uint16>    div       1048576 per        0.186199sec ->   5 Mops/sec
blockbinary<64,uint32>    div       1048576 per        0.155622sec ->   6 Mops/sec
blockbinary<64,uint64>    div       1048576 per       0.0038418sec -> 272 Mops/sec
blockbinary<128,uint8>    div        524288 per        0.223554sec ->   2 Mops/sec
blockbinary<128,uint16>   div        524288 per        0.121348sec ->   4 Mops/sec
blockbinary<128,uint32>   div        524288 per        0.114737sec ->   4 Mops/sec
blockbinary<256,uint8>    div        262144 per        0.190123sec ->   1 Mops/sec
blockbinary<256,uint16>   div        262144 per        0.108935sec ->   2 Mops/sec
blockbinary<256,uint32>   div        262144 per       0.0579756sec ->   4 Mops/sec
blockbinary<512,uint8>    div        131072 per        0.172796sec -> 758 Kops/sec
blockbinary<512,uint16>   div        131072 per       0.0992379sec ->   1 Mops/sec
blockbinary<512,uint32>   div        131072 per       0.0543442sec ->   2 Mops/sec
blockbinary<1024,uint8>   div         65536 per        0.166481sec -> 393 Kops/sec
blockbinary<1024,uint16>  div         65536 per        0.104359sec -> 627 Kops/sec
blockbinary<1024,uint32>  div         65536 per       0.0671245sec -> 976 Kops/sec

REMAINDER: blockbinary arithemetic performance as a function of size and BlockType
blockbinary<4,uint8>      rem       1048576 per       0.0114458sec ->  91 Mops/sec
blockbinary<8,uint8>      rem       1048576 per       0.0036695sec -> 285 Mops/sec
blockbinary<16,uint8>     rem       1048576 per       0.0375856sec ->  27 Mops/sec
blockbinary<16,uint16>    rem       1048576 per         0.00364sec -> 288 Mops/sec
blockbinary<32,uint8>     rem       1048576 per        0.043568sec ->  24 Mops/sec
blockbinary<32,uint16>    rem       1048576 per       0.0366663sec ->  28 Mops/sec
blockbinary<32,uint32>    rem       1048576 per        0.003658sec -> 286 Mops/sec
blockbinary<64,uint8>     rem       1048576 per       0.0680727sec ->  15 Mops/sec
blockbinary<64,uint16>    rem       1048576 per       0.0565005sec ->  18 Mops/sec
blockbinary<64,uint32>    rem       1048576 per       0.0505405sec ->  20 Mops/sec
blockbinary<64,uint64>    rem       1048576 per       0.0036401sec -> 288 Mops/sec
blockbinary<128,uint8>    rem        524288 per       0.0553031sec ->   9 Mops/sec
blockbinary<128,uint16>   rem        524288 per       0.0349244sec ->  15 Mops/sec
blockbinary<128,uint32>   rem        524288 per       0.0294627sec ->  17 Mops/sec
blockbinary<256,uint8>    rem        262144 per        0.065469sec ->   4 Mops/sec
blockbinary<256,uint16>   rem        262144 per       0.0326798sec ->   8 Mops/sec
blockbinary<256,uint32>   rem        262144 per       0.0164991sec ->  15 Mops/sec
blockbinary<512,uint8>    rem        131072 per       0.0483153sec ->   2 Mops/sec
blockbinary<512,uint16>   rem        131072 per       0.0231198sec ->   5 Mops/sec
blockbinary<512,uint32>   rem        131072 per       0.0191297sec ->   6 Mops/sec
blockbinary<1024,uint8>   rem         65536 per       0.0434145sec ->   1 Mops/sec
blockbinary<1024,uint16>  rem         65536 per       0.0287092sec ->   2 Mops/sec
blockbinary<1024,uint32>  rem         65536 per       0.0135218sec ->   4 Mops/sec
*/