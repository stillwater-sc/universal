//  performance.cpp : performance benchmarking for blockfraction arithmetic
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

#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/benchmark/performance_runner.hpp>

// test construction peformance
void TestBlockPerformanceOnConstruction() {
	using namespace sw::universal;
	std::cout << "\nConstruction performance\n";

	constexpr size_t NR_OPS = 1024ull * 1024ull + 1;

	PerformanceRunner("blockfraction<8>    construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16>   construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32>   construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64>   construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<64, uint64_t> >, NR_OPS);
	PerformanceRunner("blockfraction<128>  construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<256>  construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<512>  construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<1024> construction  ", ConstructionPerformanceWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS);
}

// test performance of shift operator on blockfraction<> class
void TestShiftOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nLogical shift operator performance\n";

	constexpr size_t NR_OPS = 1024ull * 1024ull;

	PerformanceRunner("blockfraction<16>   shifts        ", ShiftPerformanceWorkload< sw::universal::blockfraction<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32>   shifts        ", ShiftPerformanceWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64>   shifts        ", ShiftPerformanceWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
#if REGRESSION_LEVEL_4
	PerformanceRunner("blockfraction<128>  shifts        ", ShiftPerformanceWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<256>  shifts        ", ShiftPerformanceWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512>  shifts        ", ShiftPerformanceWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024> shifts        ", ShiftPerformanceWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);
#endif
}

void TestBlockPerformanceOnShift() {
	using namespace sw::universal;
	std::cout << "\nBlock size performance on logical shift operators\n";

	constexpr size_t NR_OPS = 1024ull * 1024ull;

	PerformanceRunner("blockfraction<8,uint8>     shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<8, uint8_t> >, NR_OPS);

	PerformanceRunner("blockfraction<16,uint8>    shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint16>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<16, uint16_t> >, NR_OPS);

	PerformanceRunner("blockfraction<32,uint8>    shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint16>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint32>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);

	PerformanceRunner("blockfraction<64,uint8>    shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint16>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint32>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
#if REGRESSION_LEVEL_4
	PerformanceRunner("blockfraction<128,uint8>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint16>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint32>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);

	PerformanceRunner("blockfraction<256,uint8>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint16>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint32>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);

	PerformanceRunner("blockfraction<512,uint8>   shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<1024, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint16>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<1024, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint32>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 8);

	PerformanceRunner("blockfraction<1024,uint8>  shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<1024,uint16> shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<1024, uint16_t> >, NR_OPS /16);
	PerformanceRunner("blockfraction<1024,uint32> shifts  ", ShiftPerformanceWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);
#endif
}

// Generic set of adds and subtracts for a given number system type
template<typename Scalar>
void BlockFractionAdditionWorkload(size_t NR_OPS) {
	constexpr size_t nbits = Scalar::nbits;
	using bt = typename Scalar::BlockType;

	sw::universal::blockfraction<nbits, bt> a, b, c, d;
	a.setradix(nbits);
	a.setbits(0xFFFF'FFFF'FFFF'FFFFull);
	a = b;
	for (size_t i = 0; i < NR_OPS; ++i) {
		c.add(a, b);
		d = c;
	}
}

template<typename Scalar>
void BlockFractionSubtractionWorkload(size_t NR_OPS) {
	constexpr size_t nbits = Scalar::nbits;
	using bt = typename Scalar::BlockType;

	sw::universal::blockfraction<nbits, bt> a, b, c, d;
	a.setradix(nbits);
	a.setbits(0xFFFF'FFFF'FFFF'FFFFull);
	a = b;
	for (size_t i = 0; i < NR_OPS; ++i) {
		c.sub(a, b);
		d = c;
	}
}

template<typename Scalar>
void BlockFractionMultiplicationWorkload(size_t NR_OPS) {
	constexpr size_t nbits = Scalar::nbits;
	using bt = typename Scalar::BlockType;

	sw::universal::blockfraction<nbits, bt> a, b;
	a.setradix(nbits);
	Scalar c, d;
	a.setbits(0xFFFF'FFFF'FFFF'FFFFull);
	a = b;
	for (size_t i = 0; i < NR_OPS; ++i) {
		c.mul(a, b);
		c.clear(); // reset to zero so d = c is fast
		d = c;
	}
}

template<typename Scalar>
void BlockFractionDivisionWorkload(size_t NR_OPS) {
	constexpr size_t nbits = Scalar::nbits;
	using bt = typename Scalar::BlockType;

	sw::universal::blockfraction<nbits, bt> a, b;
	a.setradix(nbits);
	Scalar c, d;
	a.setbits(0xFFFF'FFFF'FFFF'FFFFull);
	a = b;
	for (size_t i = 0; i < NR_OPS; ++i) {
		c.div(a, b);
		c.clear(); // reset to zero so d = c is fast
		d = c;
	}
}

void TestArithmeticOperatorPerformance() {
	using namespace sw::universal;
	std::cout << "\nArithmetic operator performance\n";

	size_t NR_OPS = 1024ull * 1024ull * 2ull;

	PerformanceRunner("blockfraction<16>   add           ", BlockFractionAdditionWorkload< sw::universal::blockfraction<16, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32>   add           ", BlockFractionAdditionWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64>   add           ", BlockFractionAdditionWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<128>  add           ", BlockFractionAdditionWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<256>  add           ", BlockFractionAdditionWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512>  add           ", BlockFractionAdditionWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024> add           ", BlockFractionAdditionWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);

	PerformanceRunner("blockfraction<16>       subtract  ", BlockFractionSubtractionWorkload< sw::universal::blockfraction<16, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32>       subtract  ", BlockFractionSubtractionWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64>       subtract  ", BlockFractionSubtractionWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<128>      subtract  ", BlockFractionSubtractionWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<256>      subtract  ", BlockFractionSubtractionWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512>      subtract  ", BlockFractionSubtractionWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024>     subtract  ", BlockFractionSubtractionWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);

	NR_OPS = 1024ull * 1024ull;
	PerformanceRunner("blockfraction<16>   multiplication", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<16, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32>   multiplication", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<64>   multiplication", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<128>  multiplication", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 64);
	PerformanceRunner("blockfraction<512>  multiplication", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 512);     // TODO: why is this so slow?
	PerformanceRunner("blockfraction<1024> multiplication", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 1024);   // TODO: why is this so slow?

	NR_OPS = 1024ull * 512ull;
	PerformanceRunner("blockfraction<16>   division      ", BlockFractionDivisionWorkload< sw::universal::blockfraction<16, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32>   division      ", BlockFractionDivisionWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64>   division      ", BlockFractionDivisionWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128>  division      ", BlockFractionDivisionWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512>  division      ", BlockFractionDivisionWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024> division      ", BlockFractionDivisionWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);

#ifdef FRACTION_REMAINDER
	NR_OPS = 1024ull * 512ull;
	PerformanceRunner("blockfraction<16>   remainder     ", RemainderWorkload< sw::universal::blockfraction<16, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32>   remainder     ", RemainderWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64>   remainder     ", RemainderWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128>  remainder     ", RemainderWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512>  remainder     ", RemainderWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024> remainder     ", RemainderWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);
#endif
}

void TestBlockPerformanceOnAdd() {
	using namespace sw::universal;
	std::cout << "\nADDITION: blockfraction arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 2ull * 1024ull * 1024ull;

	PerformanceRunner("blockfraction<4,uint8>      add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<8,uint8>      add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint8>     add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint16>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint8>     add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint16>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint32>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint8>     add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint16>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint32>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
#if REGRESSION_LEVEL_4
	PerformanceRunner("blockfraction<128,uint8>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint16>   add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint32>   add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<256,uint8>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint16>   add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint32>   add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512,uint8>    add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint16>   add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint32>   add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024,uint8>   add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<1024,uint16>  add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<1024,uint32>  add   ", BlockFractionAdditionWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);
#endif
}

void TestBlockPerformanceOnDiv() {
	using namespace sw::universal;
	std::cout << "\nDIVISION: blockfraction arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 1024ull * 1024;

	PerformanceRunner("blockfraction<4,uint8>      div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<8,uint8>      div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint8>     div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint16>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint8>     div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint16>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint32>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint8>     div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint16>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint32>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
#if REGRESSION_LEVEL_4
	PerformanceRunner("blockfraction<128,uint8>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint16>   div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint32>   div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<256,uint8>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint16>   div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint32>   div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512,uint8>    div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint16>   div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint32>   div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024,uint8>   div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<1024,uint16>  div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<1024,uint32>  div   ", BlockFractionDivisionWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);
#endif
}

#ifdef FRACTION_REMAINDER
void TestBlockPerformanceOnRem() {
	using namespace sw::universal;
	std::cout << "\nREMAINDER: blockfraction arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 1024ull * 1024;
	constexpr BitEncoding ones = BitEncoding::Ones;

	PerformanceRunner("blockfraction<4,uint8>      rem   ", RemainderWorkload< sw::universal::blockfraction<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<8,uint8>      rem   ", RemainderWorkload< sw::universal::blockfraction<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint8>     rem   ", RemainderWorkload< sw::universal::blockfraction<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint16>    rem   ", RemainderWorkload< sw::universal::blockfraction<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint8>     rem   ", RemainderWorkload< sw::universal::blockfraction<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint16>    rem   ", RemainderWorkload< sw::universal::blockfraction<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint32>    rem   ", RemainderWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint8>     rem   ", RemainderWorkload< sw::universal::blockfraction<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint16>    rem   ", RemainderWorkload< sw::universal::blockfraction<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint32>    rem   ", RemainderWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<128,uint8>    rem   ", RemainderWorkload< sw::universal::blockfraction<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint16>   rem   ", RemainderWorkload< sw::universal::blockfraction<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint32>   rem   ", RemainderWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<256,uint8>    rem   ", RemainderWorkload< sw::universal::blockfraction<256, uint8_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint16>   rem   ", RemainderWorkload< sw::universal::blockfraction<256, uint16_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<256,uint32>   rem   ", RemainderWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512,uint8>    rem   ", RemainderWorkload< sw::universal::blockfraction<512, uint8_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint16>   rem   ", RemainderWorkload< sw::universal::blockfraction<512, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<512,uint32>   rem   ", RemainderWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<1024,uint8>   rem   ", RemainderWorkload< sw::universal::blockfraction<1024, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<1024,uint16>  rem   ", RemainderWorkload< sw::universal::blockfraction<1024, uint16_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<1024,uint32>  rem   ", RemainderWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 16);
}
#endif

void TestBlockPerformanceOnMul() {
	using namespace sw::universal;
	std::cout << "\nMULTIPLICATION: blockfraction arithemetic performance as a function of size and BlockType\n";

	constexpr size_t NR_OPS = 512ull * 1024;

	PerformanceRunner("blockfraction<4,uint8>      mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<4, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<8,uint8>      mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<8, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint8>     mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<16, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<16,uint16>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<16, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint8>     mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<32, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint16>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<32, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<32,uint32>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<32, uint32_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint8>     mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<64, uint8_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint16>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<64, uint16_t> >, NR_OPS);
	PerformanceRunner("blockfraction<64,uint32>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<64, uint32_t> >, NR_OPS);
#if REGRESSION_LEVEL_4
	PerformanceRunner("blockfraction<128,uint8>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<128, uint8_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint16>   mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<128, uint16_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<128,uint32>   mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<128, uint32_t> >, NR_OPS / 2);
	PerformanceRunner("blockfraction<256,uint8>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<256, uint8_t> >, NR_OPS / 16);
	PerformanceRunner("blockfraction<256,uint16>   mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<256, uint16_t> >, NR_OPS / 8);
	PerformanceRunner("blockfraction<256,uint32>   mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<256, uint32_t> >, NR_OPS / 4);
	PerformanceRunner("blockfraction<512,uint8>    mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<512, uint8_t> >, NR_OPS / 512);
	PerformanceRunner("blockfraction<512,uint16>   mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<512, uint16_t> >, NR_OPS / 256);
	PerformanceRunner("blockfraction<512,uint32>   mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<512, uint32_t> >, NR_OPS / 128);
	PerformanceRunner("blockfraction<1024,uint8>   mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<1024, uint8_t> >, NR_OPS / 1024);
	PerformanceRunner("blockfraction<1024,uint16>  mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<1024, uint16_t> >, NR_OPS / 512);
	PerformanceRunner("blockfraction<1024,uint32>  mul   ", BlockFractionMultiplicationWorkload< sw::universal::blockfraction<1024, uint32_t> >, NR_OPS / 256);
#endif
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

	std::string test_suite  = "blockfraction operator performance benchmarking";
	std::string test_tag    = "blockfraction performance";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	ShiftPerformanceWorkload< sw::universal::blockfraction<8, uint8_t> >(1);

	TestBlockPerformanceOnConstruction();
	TestBlockPerformanceOnShift();
	TestBlockPerformanceOnAdd();
	TestBlockPerformanceOnMul();
	// these are long running tests due to the fact that blockfraction div is slow for large configurations
	TestBlockPerformanceOnDiv();
#ifdef FRACTION_REMAINDER
	TestBlockPerformanceOnRem();
#endif
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();

	ShiftPerformanceWorkload< sw::universal::blockfraction<8, uint8_t> >(1);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
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

blockfraction operator performance benchmarking

Logical shift operator performance
blockfraction<16>   shifts            1000000 per       0.0013967sec -> 715 Mops/sec
blockfraction<32>   shifts            1000000 per       0.0063932sec -> 156 Mops/sec
blockfraction<64>   shifts            1000000 per       0.0210088sec ->  47 Mops/sec
blockfraction<128>  shifts             500000 per       0.0186187sec ->  26 Mops/sec
blockfraction<256>  shifts             250000 per       0.0265821sec ->   9 Mops/sec
blockfraction<512>  shifts             125000 per       0.0387919sec ->   3 Mops/sec
blockfraction<1024> shifts              62500 per       0.0252466sec ->   2 Mops/sec

Arithmetic operator performance
blockfraction<16>   add/subtract      1000000 per       0.0129235sec ->  77 Mops/sec
blockfraction<32>   add/subtract      1000000 per       0.0212104sec ->  47 Mops/sec
blockfraction<64>   add/subtract      1000000 per       0.0308646sec ->  32 Mops/sec
blockfraction<128>  add/subtract       500000 per       0.0303105sec ->  16 Mops/sec
blockfraction<256>  add/subtract       250000 per       0.0308254sec ->   8 Mops/sec
blockfraction<512>  add/subtract       125000 per       0.0354519sec ->   3 Mops/sec
blockfraction<1024> add/subtract        62500 per       0.0366506sec ->   1 Mops/sec
blockfraction<16>   division            32768 per       0.0022338sec ->  14 Mops/sec
blockfraction<32>   division            32768 per       0.0028381sec ->  11 Mops/sec
blockfraction<64>   division            16384 per       0.0024436sec ->   6 Mops/sec
blockfraction<128>  division             8192 per       0.0024895sec ->   3 Mops/sec
blockfraction<512>  division             4096 per       0.0060114sec -> 681 Kops/sec
blockfraction<1024> division             2048 per       0.0067702sec -> 302 Kops/sec
blockfraction<16>   remainder           32768 per       0.0025087sec ->  13 Mops/sec
blockfraction<32>   remainder           32768 per       0.0034877sec ->   9 Mops/sec
blockfraction<64>   remainder           16384 per       0.0031411sec ->   5 Mops/sec
blockfraction<128>  remainder            8192 per       0.0026535sec ->   3 Mops/sec
blockfraction<512>  remainder            4096 per       0.0071541sec -> 572 Kops/sec
blockfraction<1024> remainder            2048 per        0.006772sec -> 302 Kops/sec
blockfraction<16>   multiplication      32768 per       0.0013644sec ->  24 Mops/sec
blockfraction<32>   multiplication      16384 per       0.0027321sec ->   5 Mops/sec
blockfraction<64>   multiplication       8192 per       0.0051987sec ->   1 Mops/sec
blockfraction<128>  multiplication       4096 per       0.0163804sec -> 250 Kops/sec
blockfraction<512>  multiplication       2048 per        0.242991sec ->   8 Kops/sec
blockfraction<1024> multiplication       1024 per        0.283926sec ->   3 Kops/sec

Block size performance on logical shift operators
blockfraction<8,uint8>     shifts      1000000 per           3e-07sec ->   3 Tops/sec
blockfraction<16,uint8>    shifts      1000000 per       0.0016431sec -> 608 Mops/sec
blockfraction<16,uint16>   shifts      1000000 per           1e-07sec ->  10 Tops/sec
blockfraction<32,uint8>    shifts      1000000 per       0.0040908sec -> 244 Mops/sec
blockfraction<32,uint16>   shifts      1000000 per       0.0019056sec -> 524 Mops/sec
blockfraction<32,uint32>   shifts      1000000 per           2e-07sec ->   5 Tops/sec
blockfraction<64,uint8>    shifts      1000000 per       0.0130328sec ->  76 Mops/sec
blockfraction<64,uint16>   shifts      1000000 per       0.0053097sec -> 188 Mops/sec
blockfraction<64,uint32>   shifts      1000000 per       0.0007268sec ->   1 Gops/sec
blockfraction<128,uint8>   shifts       500000 per       0.0146397sec ->  34 Mops/sec
blockfraction<128,uint16>  shifts       500000 per       0.0050691sec ->  98 Mops/sec
blockfraction<128,uint32>  shifts       500000 per       0.0022174sec -> 225 Mops/sec
blockfraction<256,uint8>   shifts       250000 per       0.0245585sec ->  10 Mops/sec
blockfraction<256,uint16>  shifts       250000 per       0.0096754sec ->  25 Mops/sec
blockfraction<256,uint32>  shifts       250000 per       0.0028712sec ->  87 Mops/sec
blockfraction<512,uint8>   shifts       125000 per       0.0418973sec ->   2 Mops/sec
blockfraction<512,uint16>  shifts       125000 per       0.0353224sec ->   3 Mops/sec
blockfraction<512,uint32>  shifts       125000 per       0.0071748sec ->  17 Mops/sec
blockfraction<1024,uint8>  shifts        62500 per       0.0248754sec ->   2 Mops/sec
blockfraction<1024,uint16> shifts        62500 per       0.0177027sec ->   3 Mops/sec
blockfraction<1024,uint32> shifts        62500 per        0.003406sec ->  18 Mops/sec

block size performance
blockfraction<4,uint8>      add       1000000 per        0.001309sec -> 763 Mops/sec
blockfraction<8,uint8>      add       1000000 per       0.0010282sec -> 972 Mops/sec
blockfraction<16,uint8>     add       1000000 per       0.0125934sec ->  79 Mops/sec
blockfraction<16,uint16>    add       1000000 per       0.0009041sec ->   1 Gops/sec
blockfraction<32,uint8>     add       1000000 per        0.019086sec ->  52 Mops/sec
blockfraction<32,uint16>    add       1000000 per       0.0143736sec ->  69 Mops/sec
blockfraction<32,uint32>    add       1000000 per       0.0006061sec ->   1 Gops/sec
blockfraction<64,uint8>     add       1000000 per        0.030433sec ->  32 Mops/sec
blockfraction<64,uint16>    add       1000000 per       0.0186879sec ->  53 Mops/sec
blockfraction<64,uint32>    add       1000000 per        0.012867sec ->  77 Mops/sec
blockfraction<128,uint8>    add        500000 per       0.0304265sec ->  16 Mops/sec
blockfraction<128,uint16>   add        500000 per        0.015848sec ->  31 Mops/sec
blockfraction<128,uint32>   add        500000 per       0.0129498sec ->  38 Mops/sec
blockfraction<256,uint8>    add        250000 per       0.0419406sec ->   5 Mops/sec
blockfraction<256,uint16>   add        250000 per       0.0171904sec ->  14 Mops/sec
blockfraction<256,uint32>   add        250000 per       0.0079432sec ->  31 Mops/sec
blockfraction<512,uint8>    add        125000 per       0.0360764sec ->   3 Mops/sec
blockfraction<512,uint16>   add        125000 per       0.0168952sec ->   7 Mops/sec
blockfraction<512,uint32>   add        125000 per       0.0068201sec ->  18 Mops/sec
blockfraction<1024,uint8>   add         62500 per       0.0372607sec ->   1 Mops/sec
blockfraction<1024,uint16>  add         62500 per       0.0183361sec ->   3 Mops/sec
blockfraction<1024,uint32>  add         62500 per       0.0084381sec ->   7 Mops/sec

block size performance
blockfraction<4,uint8>      mul        500000 per        0.006492sec ->  77 Mops/sec
blockfraction<8,uint8>      mul        500000 per       0.0084224sec ->  59 Mops/sec
blockfraction<16,uint8>     mul        500000 per       0.0196726sec ->  25 Mops/sec
blockfraction<16,uint16>    mul        500000 per        0.009335sec ->  53 Mops/sec
blockfraction<32,uint8>     mul        500000 per       0.0685858sec ->   7 Mops/sec
blockfraction<32,uint16>    mul        500000 per        0.045748sec ->  10 Mops/sec
blockfraction<32,uint32>    mul        500000 per       0.0264499sec ->  18 Mops/sec
blockfraction<64,uint8>     mul        500000 per        0.310805sec ->   1 Mops/sec
blockfraction<64,uint16>    mul        500000 per        0.152077sec ->   3 Mops/sec
blockfraction<64,uint32>    mul        500000 per       0.0731226sec ->   6 Mops/sec
blockfraction<128,uint8>    mul        250000 per        0.758251sec -> 329 Kops/sec
blockfraction<128,uint16>   mul        250000 per         0.31448sec -> 794 Kops/sec
blockfraction<128,uint32>   mul        250000 per        0.147211sec ->   1 Mops/sec
blockfraction<256,uint8>    mul         62500 per         1.05118sec ->  59 Kops/sec
blockfraction<256,uint16>   mul        125000 per        0.755691sec -> 165 Kops/sec
blockfraction<256,uint32>   mul        125000 per        0.373575sec -> 334 Kops/sec
blockfraction<512,uint8>    mul         15625 per          1.8431sec ->   8 Kops/sec
blockfraction<512,uint16>   mul         31250 per         1.03459sec ->  30 Kops/sec
blockfraction<512,uint32>   mul         62500 per        0.760108sec ->  82 Kops/sec
blockfraction<1024,uint8>   mul          7812 per         2.26072sec ->   3 Kops/sec
blockfraction<1024,uint16>  mul         15625 per         3.68005sec ->   4 Kops/sec
blockfraction<1024,uint32>  mul         31250 per         2.08747sec ->  14 Kops/sec

block size performance
blockfraction<4,uint8>      div       1000000 per       0.0227702sec ->  43 Mops/sec
blockfraction<8,uint8>      div       1000000 per       0.0581431sec ->  17 Mops/sec
blockfraction<16,uint8>     div       1000000 per       0.0693781sec ->  14 Mops/sec
blockfraction<16,uint16>    div       1000000 per       0.0742125sec ->  13 Mops/sec
blockfraction<32,uint8>     div       1000000 per        0.088973sec ->  11 Mops/sec
blockfraction<32,uint16>    div       1000000 per       0.0700939sec ->  14 Mops/sec
blockfraction<32,uint32>    div       1000000 per       0.0728259sec ->  13 Mops/sec
blockfraction<64,uint8>     div       1000000 per        0.149565sec ->   6 Mops/sec
blockfraction<64,uint16>    div       1000000 per       0.0992803sec ->  10 Mops/sec
blockfraction<64,uint32>    div       1000000 per       0.0781339sec ->  12 Mops/sec
blockfraction<128,uint8>    div        500000 per        0.153718sec ->   3 Mops/sec
blockfraction<128,uint16>   div        500000 per       0.0874746sec ->   5 Mops/sec
blockfraction<128,uint32>   div        500000 per       0.0541529sec ->   9 Mops/sec
blockfraction<256,uint8>    div        250000 per        0.170179sec ->   1 Mops/sec
blockfraction<256,uint16>   div        250000 per       0.0835185sec ->   2 Mops/sec
blockfraction<256,uint32>   div        250000 per       0.0421212sec ->   5 Mops/sec
blockfraction<512,uint8>    div        125000 per        0.185005sec -> 675 Kops/sec
blockfraction<512,uint16>   div        125000 per       0.0944967sec ->   1 Mops/sec
blockfraction<512,uint32>   div        125000 per       0.0419085sec ->   2 Mops/sec
blockfraction<1024,uint8>   div         62500 per        0.191113sec -> 327 Kops/sec
blockfraction<1024,uint16>  div         62500 per       0.0971181sec -> 643 Kops/sec
blockfraction<1024,uint32>  div         62500 per       0.0517993sec ->   1 Mops/sec

block size performance
blockfraction<4,uint8>      rem       1000000 per       0.0276357sec ->  36 Mops/sec
blockfraction<8,uint8>      rem       1000000 per       0.0594847sec ->  16 Mops/sec
blockfraction<16,uint8>     rem       1000000 per       0.0680941sec ->  14 Mops/sec
blockfraction<16,uint16>    rem       1000000 per       0.0677176sec ->  14 Mops/sec
blockfraction<32,uint8>     rem       1000000 per       0.0971664sec ->  10 Mops/sec
blockfraction<32,uint16>    rem       1000000 per       0.0718622sec ->  13 Mops/sec
blockfraction<32,uint32>    rem       1000000 per        0.071596sec ->  13 Mops/sec
blockfraction<64,uint8>     rem       1000000 per        0.155253sec ->   6 Mops/sec
blockfraction<64,uint16>    rem       1000000 per       0.0999466sec ->  10 Mops/sec
blockfraction<64,uint32>    rem       1000000 per       0.0821133sec ->  12 Mops/sec
blockfraction<128,uint8>    rem        500000 per        0.159737sec ->   3 Mops/sec
blockfraction<128,uint16>   rem        500000 per       0.0863155sec ->   5 Mops/sec
blockfraction<128,uint32>   rem        500000 per       0.0596871sec ->   8 Mops/sec
blockfraction<256,uint8>    rem        250000 per        0.169751sec ->   1 Mops/sec
blockfraction<256,uint16>   rem        250000 per        0.093173sec ->   2 Mops/sec
blockfraction<256,uint32>   rem        250000 per       0.0412379sec ->   6 Mops/sec
blockfraction<512,uint8>    rem        125000 per        0.189013sec -> 661 Kops/sec
blockfraction<512,uint16>   rem        125000 per       0.0946558sec ->   1 Mops/sec
blockfraction<512,uint32>   rem        125000 per       0.0427583sec ->   2 Mops/sec
blockfraction<1024,uint8>   rem         62500 per        0.194026sec -> 322 Kops/sec
blockfraction<1024,uint16>  rem         62500 per       0.0972668sec -> 642 Kops/sec
blockfraction<1024,uint32>  rem         62500 per       0.0481189sec ->   1 Mops/sec
 */

/*
ETLO
Date run : 03/01/2021
Processor: Intel Core i7-9850H CPU @ 2.60GHz, 6 cores, 12 threads, 15W mobile processor
Cache    : L1 384KB, L2 1.5MB, L3 12.0MB
Memory   : 16GB
System   : 64-bit Windows 10 Pro, Version 2004, x64-based processor, OS build 19041.804
blockfraction operator performance benchmarking

Logical shift operator performance
blockfraction<16>   shifts            1048576 per       0.0151422sec ->  69 Mops/sec
blockfraction<32>   shifts            1048576 per       0.0169131sec ->  61 Mops/sec
blockfraction<64>   shifts            1048576 per         0.02377sec ->  44 Mops/sec
blockfraction<128>  shifts             524288 per       0.0170116sec ->  30 Mops/sec
blockfraction<256>  shifts             262144 per        0.023649sec ->  11 Mops/sec
blockfraction<512>  shifts             131072 per        0.017104sec ->   7 Mops/sec
blockfraction<1024> shifts              65536 per       0.0168191sec ->   3 Mops/sec

Arithmetic operator performance
blockfraction<16>   add/subtract      2097152 per       0.0239446sec ->  87 Mops/sec
blockfraction<32>   add/subtract      2097152 per       0.0317233sec ->  66 Mops/sec
blockfraction<64>   add/subtract      2097152 per       0.0522965sec ->  40 Mops/sec
blockfraction<128>  add/subtract      1048576 per       0.0609575sec ->  17 Mops/sec
blockfraction<256>  add/subtract       524288 per       0.0612585sec ->   8 Mops/sec
blockfraction<512>  add/subtract       262144 per       0.0649979sec ->   4 Mops/sec
blockfraction<1024> add/subtract       131072 per        0.065752sec ->   1 Mops/sec
blockfraction<16>   multiplication    1048576 per       0.0294896sec ->  35 Mops/sec
blockfraction<32>   multiplication     524288 per       0.0524751sec ->   9 Mops/sec
blockfraction<64>   multiplication     262144 per        0.121624sec ->   2 Mops/sec
blockfraction<128>  multiplication      16384 per       0.0313895sec -> 521 Kops/sec
blockfraction<512>  multiplication       2048 per       0.0673462sec ->  30 Kops/sec
blockfraction<1024> multiplication       1024 per       0.0913936sec ->  11 Kops/sec
blockfraction<16>   division           524288 per       0.0210676sec ->  24 Mops/sec
blockfraction<32>   division           524288 per       0.0326344sec ->  16 Mops/sec
blockfraction<64>   division           262144 per       0.0207816sec ->  12 Mops/sec
blockfraction<128>  division           131072 per       0.0166951sec ->   7 Mops/sec
blockfraction<512>  division            65536 per       0.0321721sec ->   2 Mops/sec
blockfraction<1024> division            32768 per       0.0352204sec -> 930 Kops/sec
blockfraction<16>   remainder          524288 per       0.0211667sec ->  24 Mops/sec
blockfraction<32>   remainder          524288 per       0.0262438sec ->  19 Mops/sec
blockfraction<64>   remainder          262144 per       0.0208688sec ->  12 Mops/sec
blockfraction<128>  remainder          131072 per       0.0150966sec ->   8 Mops/sec
blockfraction<512>  remainder           65536 per       0.0338635sec ->   1 Mops/sec
blockfraction<1024> remainder           32768 per       0.0372232sec -> 880 Kops/sec

Construction performance
blockfraction<8>    construction      1048577 per       0.0006647sec ->   1 Gops/sec
blockfraction<16>   construction      1048577 per       0.0008141sec ->   1 Gops/sec
blockfraction<32>   construction      1048577 per       0.0006457sec ->   1 Gops/sec
blockfraction<64>   construction      1048577 per       0.0005499sec ->   1 Gops/sec
blockfraction<128>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockfraction<256>  construction      1048577 per           1e-07sec ->  10 Tops/sec
blockfraction<512>  construction      1048577 per       0.0060603sec -> 173 Mops/sec
blockfraction<1024> construction      1048577 per       0.0093635sec -> 111 Mops/sec

Block size performance on logical shift operators
blockfraction<8,uint8>     shifts      1048576 per           2e-07sec ->   5 Tops/sec
blockfraction<16,uint8>    shifts      1048576 per        0.014898sec ->  70 Mops/sec
blockfraction<16,uint16>   shifts      1048576 per       0.0141619sec ->  74 Mops/sec
blockfraction<32,uint8>    shifts      1048576 per       0.0177451sec ->  59 Mops/sec
blockfraction<32,uint16>   shifts      1048576 per       0.0154789sec ->  67 Mops/sec
blockfraction<32,uint32>   shifts      1048576 per       0.0138923sec ->  75 Mops/sec
blockfraction<64,uint8>    shifts      1048576 per       0.0319717sec ->  32 Mops/sec
blockfraction<64,uint16>   shifts      1048576 per       0.0178939sec ->  58 Mops/sec
blockfraction<64,uint32>   shifts      1048576 per       0.0148067sec ->  70 Mops/sec
blockfraction<128,uint8>   shifts       524288 per       0.0172914sec ->  30 Mops/sec
blockfraction<128,uint16>  shifts       524288 per         0.01156sec ->  45 Mops/sec
blockfraction<128,uint32>  shifts       524288 per       0.0087054sec ->  60 Mops/sec
blockfraction<256,uint8>   shifts       262144 per       0.0236265sec ->  11 Mops/sec
blockfraction<256,uint16>  shifts       262144 per       0.0079614sec ->  32 Mops/sec
blockfraction<256,uint32>  shifts       262144 per        0.005102sec ->  51 Mops/sec
blockfraction<512,uint8>   shifts       131072 per       0.0326855sec ->   4 Mops/sec
blockfraction<512,uint16>  shifts       131072 per       0.0179826sec ->   7 Mops/sec
blockfraction<512,uint32>  shifts       131072 per       0.0083022sec ->  15 Mops/sec
blockfraction<1024,uint8>  shifts        65536 per       0.0165172sec ->   3 Mops/sec
blockfraction<1024,uint16> shifts        65536 per       0.0083414sec ->   7 Mops/sec
blockfraction<1024,uint32> shifts        65536 per       0.0043763sec ->  14 Mops/sec

ADDITION: blockfraction arithemetic performance as a function of size and BlockType
blockfraction<4,uint8>      add       2097152 per       0.0005575sec ->   3 Gops/sec
blockfraction<8,uint8>      add       2097152 per               0sec ->   0  ops/sec
blockfraction<16,uint8>     add       2097152 per       0.0228691sec ->  91 Mops/sec
blockfraction<16,uint16>    add       2097152 per           1e-07sec ->  20 Tops/sec
blockfraction<32,uint8>     add       2097152 per       0.0321724sec ->  65 Mops/sec
blockfraction<32,uint16>    add       2097152 per       0.0238562sec ->  87 Mops/sec
blockfraction<32,uint32>    add       2097152 per           1e-07sec ->  20 Tops/sec
blockfraction<64,uint8>     add       2097152 per       0.0522177sec ->  40 Mops/sec
blockfraction<64,uint16>    add       2097152 per       0.0314089sec ->  66 Mops/sec
blockfraction<64,uint32>    add       2097152 per       0.0240764sec ->  87 Mops/sec
blockfraction<128,uint8>    add       1048576 per        0.061275sec ->  17 Mops/sec
blockfraction<128,uint16>   add       1048576 per       0.0229437sec ->  45 Mops/sec
blockfraction<128,uint32>   add       1048576 per       0.0113588sec ->  92 Mops/sec
blockfraction<256,uint8>    add        524288 per       0.0584427sec ->   8 Mops/sec
blockfraction<256,uint16>   add        524288 per       0.0260387sec ->  20 Mops/sec
blockfraction<256,uint32>   add        524288 per       0.0121753sec ->  43 Mops/sec
blockfraction<512,uint8>    add        262144 per        0.063064sec ->   4 Mops/sec
blockfraction<512,uint16>   add        262144 per       0.0289259sec ->   9 Mops/sec
blockfraction<512,uint32>   add        262144 per       0.0135417sec ->  19 Mops/sec
blockfraction<1024,uint8>   add        131072 per        0.068664sec ->   1 Mops/sec
blockfraction<1024,uint16>  add        131072 per       0.0332106sec ->   3 Mops/sec
blockfraction<1024,uint32>  add        131072 per       0.0153358sec ->   8 Mops/sec

MULTIPLICATION: blockfraction arithemetic performance as a function of size and BlockType
blockfraction<4,uint8>      mul        524288 per       0.0021491sec -> 243 Mops/sec
blockfraction<8,uint8>      mul        524288 per       0.0048791sec -> 107 Mops/sec
blockfraction<16,uint8>     mul        524288 per       0.0147906sec ->  35 Mops/sec
blockfraction<16,uint16>    mul        524288 per       0.0104389sec ->  50 Mops/sec
blockfraction<32,uint8>     mul        524288 per       0.0529245sec ->   9 Mops/sec
blockfraction<32,uint16>    mul        524288 per       0.0291708sec ->  17 Mops/sec
blockfraction<32,uint32>    mul        524288 per       0.0145993sec ->  35 Mops/sec
blockfraction<64,uint8>     mul        524288 per        0.240083sec ->   2 Mops/sec
blockfraction<64,uint16>    mul        524288 per        0.114896sec ->   4 Mops/sec
blockfraction<64,uint32>    mul        524288 per       0.0577882sec ->   9 Mops/sec
blockfraction<128,uint8>    mul        262144 per        0.501657sec -> 522 Kops/sec
blockfraction<128,uint16>   mul        262144 per        0.186017sec ->   1 Mops/sec
blockfraction<128,uint32>   mul        262144 per       0.0887587sec ->   2 Mops/sec
blockfraction<256,uint8>    mul         32768 per        0.305953sec -> 107 Kops/sec
blockfraction<256,uint16>   mul         65536 per        0.188149sec -> 348 Kops/sec
blockfraction<256,uint32>   mul        131072 per        0.135927sec -> 964 Kops/sec
blockfraction<512,uint8>    mul          1024 per       0.0329513sec ->  31 Kops/sec
blockfraction<512,uint16>   mul          2048 per       0.0342709sec ->  59 Kops/sec
blockfraction<512,uint32>   mul          4096 per       0.0206824sec -> 198 Kops/sec
blockfraction<1024,uint8>   mul           512 per       0.0422902sec ->  12 Kops/sec
blockfraction<1024,uint16>  mul          1024 per       0.0618386sec ->  16 Kops/sec
blockfraction<1024,uint32>  mul          2048 per       0.0635144sec ->  32 Kops/sec

DIVISION: blockfraction arithemetic performance as a function of size and BlockType
blockfraction<4,uint8>      div       1048576 per       0.0174991sec ->  59 Mops/sec
blockfraction<8,uint8>      div       1048576 per       0.0276323sec ->  37 Mops/sec
blockfraction<16,uint8>     div       1048576 per       0.0426758sec ->  24 Mops/sec
blockfraction<16,uint16>    div       1048576 per       0.0421559sec ->  24 Mops/sec
blockfraction<32,uint8>     div       1048576 per       0.0528577sec ->  19 Mops/sec
blockfraction<32,uint16>    div       1048576 per       0.0422172sec ->  24 Mops/sec
blockfraction<32,uint32>    div       1048576 per       0.0424791sec ->  24 Mops/sec
blockfraction<64,uint8>     div       1048576 per       0.0832279sec ->  12 Mops/sec
blockfraction<64,uint16>    div       1048576 per       0.0575975sec ->  18 Mops/sec
blockfraction<64,uint32>    div       1048576 per       0.0533793sec ->  19 Mops/sec
blockfraction<128,uint8>    div        524288 per       0.0579575sec ->   9 Mops/sec
blockfraction<128,uint16>   div        524288 per       0.0287337sec ->  18 Mops/sec
blockfraction<128,uint32>   div        524288 per       0.0215491sec ->  24 Mops/sec
blockfraction<256,uint8>    div        262144 per       0.0597042sec ->   4 Mops/sec
blockfraction<256,uint16>   div        262144 per       0.0312451sec ->   8 Mops/sec
blockfraction<256,uint32>   div        262144 per       0.0148073sec ->  17 Mops/sec
blockfraction<512,uint8>    div        131072 per       0.0641508sec ->   2 Mops/sec
blockfraction<512,uint16>   div        131072 per       0.0325873sec ->   4 Mops/sec
blockfraction<512,uint32>   div        131072 per       0.0168981sec ->   7 Mops/sec
blockfraction<1024,uint8>   div         65536 per       0.0724597sec -> 904 Kops/sec
blockfraction<1024,uint16>  div         65536 per       0.0375644sec ->   1 Mops/sec
blockfraction<1024,uint32>  div         65536 per        0.017201sec ->   3 Mops/sec

REMAINDER: blockfraction arithemetic performance as a function of size and BlockType
blockfraction<4,uint8>      rem       1048576 per       0.0174541sec ->  60 Mops/sec
blockfraction<8,uint8>      rem       1048576 per       0.0279139sec ->  37 Mops/sec
blockfraction<16,uint8>     rem       1048576 per       0.0430577sec ->  24 Mops/sec
blockfraction<16,uint16>    rem       1048576 per       0.0418485sec ->  25 Mops/sec
blockfraction<32,uint8>     rem       1048576 per       0.0523426sec ->  20 Mops/sec
blockfraction<32,uint16>    rem       1048576 per       0.0417373sec ->  25 Mops/sec
blockfraction<32,uint32>    rem       1048576 per       0.0427313sec ->  24 Mops/sec
blockfraction<64,uint8>     rem       1048576 per       0.0834139sec ->  12 Mops/sec
blockfraction<64,uint16>    rem       1048576 per       0.0584555sec ->  17 Mops/sec
blockfraction<64,uint32>    rem       1048576 per       0.0552587sec ->  18 Mops/sec
blockfraction<128,uint8>    rem        524288 per       0.0585331sec ->   8 Mops/sec
blockfraction<128,uint16>   rem        524288 per        0.029061sec ->  18 Mops/sec
blockfraction<128,uint32>   rem        524288 per       0.0209504sec ->  25 Mops/sec
blockfraction<256,uint8>    rem        262144 per       0.0592536sec ->   4 Mops/sec
blockfraction<256,uint16>   rem        262144 per        0.031582sec ->   8 Mops/sec
blockfraction<256,uint32>   rem        262144 per       0.0144546sec ->  18 Mops/sec
blockfraction<512,uint8>    rem        131072 per       0.0645029sec ->   2 Mops/sec
blockfraction<512,uint16>   rem        131072 per       0.0332093sec ->   3 Mops/sec
blockfraction<512,uint32>   rem        131072 per       0.0172867sec ->   7 Mops/sec
blockfraction<1024,uint8>   rem         65536 per       0.0726507sec -> 902 Kops/sec
blockfraction<1024,uint16>  rem         65536 per       0.0391768sec ->   1 Mops/sec
blockfraction<1024,uint32>  rem         65536 per       0.0170348sec ->   3 Mops/sec
*/