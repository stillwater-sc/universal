// special_cases.cpp: application programming interface tests for special cases areal number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/number/areal/table.hpp>

template<size_t nbits, size_t es, typename bt = uint8_t>
inline int TestZero() {
	int fails = 0;
	sw::universal::areal<nbits, es, bt> r;
	//std::cout << to_binary(r) << std::endl;
	if (!r.iszero()) ++fails;
	r = -r;
	//std::cout << to_binary(r) << std::endl;
	if (!r.iszero()) ++fails;
	return fails;
}

void TestIsZero(int& nrOfFailedTestCases) {
	int currentFails = nrOfFailedTestCases;
	std::cout << "iszero()                       : ";
	// one block configurations
	nrOfFailedTestCases += TestZero<4, 1>();
	nrOfFailedTestCases += TestZero<5, 1>();
	nrOfFailedTestCases += TestZero<6, 1>();
	nrOfFailedTestCases += TestZero<7, 1>();
	nrOfFailedTestCases += TestZero<8, 1>();
	nrOfFailedTestCases += TestZero<8, 2>();
	nrOfFailedTestCases += TestZero<8, 3>();

	// two block configurations
	nrOfFailedTestCases += TestZero<9, 3>();
	nrOfFailedTestCases += TestZero<10, 3>();
	nrOfFailedTestCases += TestZero<11, 3>();
	nrOfFailedTestCases += TestZero<12, 3>();
	nrOfFailedTestCases += TestZero<13, 3>();
	nrOfFailedTestCases += TestZero<14, 3>();
	nrOfFailedTestCases += TestZero<15, 3>();
	nrOfFailedTestCases += TestZero<16, 3>();
	nrOfFailedTestCases += TestZero<16, 4>();
	nrOfFailedTestCases += TestZero<16, 5>();

	// three block configurations
	nrOfFailedTestCases += TestZero<17, 5>();
	nrOfFailedTestCases += TestZero<18, 5>();
	nrOfFailedTestCases += TestZero<19, 5>();
	nrOfFailedTestCases += TestZero<20, 5>();
	nrOfFailedTestCases += TestZero<21, 5>();
	nrOfFailedTestCases += TestZero<22, 5>();
	nrOfFailedTestCases += TestZero<23, 5>();
	nrOfFailedTestCases += TestZero<24, 5>();
	nrOfFailedTestCases += TestZero<24, 6>();
	nrOfFailedTestCases += TestZero<24, 7>();

	// four block configurations
	nrOfFailedTestCases += TestZero<25, 8>();
	nrOfFailedTestCases += TestZero<26, 8>();
	nrOfFailedTestCases += TestZero<27, 8>();
	nrOfFailedTestCases += TestZero<28, 8>();
	nrOfFailedTestCases += TestZero<29, 8>();
	nrOfFailedTestCases += TestZero<30, 8>();
	nrOfFailedTestCases += TestZero<31, 8>();
	nrOfFailedTestCases += TestZero<32, 8>();

	// five block configurations
	nrOfFailedTestCases += TestZero<39, 8>();
	nrOfFailedTestCases += TestZero<40, 8>();

	// six block configurations
	nrOfFailedTestCases += TestZero<47, 9>();
	nrOfFailedTestCases += TestZero<48, 9>();

	// seven block configurations
	nrOfFailedTestCases += TestZero<55, 10>();
	nrOfFailedTestCases += TestZero<56, 10>();

	// eight block configurations
	nrOfFailedTestCases += TestZero<63, 11>();
	nrOfFailedTestCases += TestZero<64, 11>();

	std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
}

template<size_t nbits, size_t es, typename bt = uint8_t>
inline int TestInf() {
	int fails = 0;
	sw::universal::areal<nbits, es, bt> r;
	r.setinf(); // default is to set -inf
	//std::cout << to_binary(r) << std::endl;
	if (!r.isinf()) ++fails;
	if (!r.isinf(sw::universal::INF_TYPE_NEGATIVE)) ++fails;
	r = -r;
	//std::cout << to_binary(r) << std::endl;
	if (!r.isinf()) ++fails;
	if (!r.isinf(sw::universal::INF_TYPE_POSITIVE)) ++fails;
	r.setnan();
	if (r.isinf()) ++fails;
	return fails;
}

void TestIsInf(int& nrOfFailedTestCases) {
	int currentFails = nrOfFailedTestCases;
	std::cout << "isinf()                        : ";
	// one block configurations
	nrOfFailedTestCases += TestInf<4, 1>();
	nrOfFailedTestCases += TestInf<5, 1>();
	nrOfFailedTestCases += TestInf<6, 1>();
	nrOfFailedTestCases += TestInf<7, 1>();
	nrOfFailedTestCases += TestInf<8, 1>();
	nrOfFailedTestCases += TestInf<8, 2>();
	nrOfFailedTestCases += TestInf<8, 3>();

	// two block configurations
	nrOfFailedTestCases += TestInf<9, 3>();
	nrOfFailedTestCases += TestInf<10, 3>();
	nrOfFailedTestCases += TestInf<11, 3>();
	nrOfFailedTestCases += TestInf<12, 3>();
	nrOfFailedTestCases += TestInf<13, 3>();
	nrOfFailedTestCases += TestInf<14, 3>();
	nrOfFailedTestCases += TestInf<15, 3>();
	nrOfFailedTestCases += TestInf<16, 3>();
	nrOfFailedTestCases += TestInf<16, 4>();
	nrOfFailedTestCases += TestInf<16, 5>();

	// three block configurations
	nrOfFailedTestCases += TestInf<17, 5>();
	nrOfFailedTestCases += TestInf<18, 5>();
	nrOfFailedTestCases += TestInf<19, 5>();
	nrOfFailedTestCases += TestInf<20, 5>();
	nrOfFailedTestCases += TestInf<21, 5>();
	nrOfFailedTestCases += TestInf<22, 5>();
	nrOfFailedTestCases += TestInf<23, 5>();
	nrOfFailedTestCases += TestInf<24, 5>();
	nrOfFailedTestCases += TestInf<24, 6>();
	nrOfFailedTestCases += TestInf<24, 7>();

	// four block configurations
	nrOfFailedTestCases += TestInf<25, 8>();
	nrOfFailedTestCases += TestInf<26, 8>();
	nrOfFailedTestCases += TestInf<27, 8>();
	nrOfFailedTestCases += TestInf<28, 8>();
	nrOfFailedTestCases += TestInf<29, 8>();
	nrOfFailedTestCases += TestInf<30, 8>();
	nrOfFailedTestCases += TestInf<31, 8>();
	nrOfFailedTestCases += TestInf<32, 8>();

	// five block configurations
	nrOfFailedTestCases += TestInf<39, 8>();
	nrOfFailedTestCases += TestInf<40, 8>();

	// six block configurations
	nrOfFailedTestCases += TestInf<47, 9>();
	nrOfFailedTestCases += TestInf<48, 9>();

	// seven block configurations
	nrOfFailedTestCases += TestInf<55, 10>();
	nrOfFailedTestCases += TestInf<56, 10>();

	// eight block configurations
	nrOfFailedTestCases += TestInf<63, 11>();
	nrOfFailedTestCases += TestInf<64, 11>();

	std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
}

template<size_t nbits, size_t es, typename bt = uint8_t>
inline int TestNaN() {
	int fails = 0;
	sw::universal::areal<nbits, es, bt> r;
	r.setnan();
	//std::cout << to_binary(r) << std::endl;
	if (!r.isnan()) ++fails;
	if (!r.isnan(sw::universal::NAN_TYPE_SIGNALLING)) ++fails;

	r = -r;
	//std::cout << to_binary(r) << std::endl;
	if (!r.isnan()) ++fails;
	if (!r.isnan(sw::universal::NAN_TYPE_QUIET)) ++fails;

	r.setinf();
	if (r.isnan()) ++fails;

	return fails;
}

void TestIsNaN(int& nrOfFailedTestCases) {
	int currentFails = nrOfFailedTestCases;
	std::cout << "isnan()                        : ";
	// one block configurations
	nrOfFailedTestCases += TestNaN<4, 1>();
	nrOfFailedTestCases += TestNaN<5, 1>();
	nrOfFailedTestCases += TestNaN<6, 1>();
	nrOfFailedTestCases += TestNaN<7, 1>();
	nrOfFailedTestCases += TestNaN<8, 1>();
	nrOfFailedTestCases += TestNaN<8, 2>();
	nrOfFailedTestCases += TestNaN<8, 3>();

	// two block configurations
	nrOfFailedTestCases += TestNaN<9, 3>();
	nrOfFailedTestCases += TestNaN<10, 3>();
	nrOfFailedTestCases += TestNaN<11, 3>();
	nrOfFailedTestCases += TestNaN<12, 3>();
	nrOfFailedTestCases += TestNaN<13, 3>();
	nrOfFailedTestCases += TestNaN<14, 3>();
	nrOfFailedTestCases += TestNaN<15, 3>();
	nrOfFailedTestCases += TestNaN<16, 3>();
	nrOfFailedTestCases += TestNaN<16, 4>();
	nrOfFailedTestCases += TestNaN<16, 5>();

	// three block configurations
	nrOfFailedTestCases += TestNaN<17, 5>();
	nrOfFailedTestCases += TestNaN<18, 5>();
	nrOfFailedTestCases += TestNaN<19, 5>();
	nrOfFailedTestCases += TestNaN<20, 5>();
	nrOfFailedTestCases += TestNaN<21, 5>();
	nrOfFailedTestCases += TestNaN<22, 5>();
	nrOfFailedTestCases += TestNaN<23, 5>();
	nrOfFailedTestCases += TestNaN<24, 5>();
	nrOfFailedTestCases += TestNaN<24, 6>();
	nrOfFailedTestCases += TestNaN<24, 7>();

	// four block configurations
	nrOfFailedTestCases += TestNaN<25, 8>();
	nrOfFailedTestCases += TestNaN<26, 8>();
	nrOfFailedTestCases += TestNaN<27, 8>();
	nrOfFailedTestCases += TestNaN<28, 8>();
	nrOfFailedTestCases += TestNaN<29, 8>();
	nrOfFailedTestCases += TestNaN<30, 8>();
	nrOfFailedTestCases += TestNaN<31, 8>();
	nrOfFailedTestCases += TestNaN<32, 8>();

	// five block configurations
	nrOfFailedTestCases += TestNaN<39, 8>();
	nrOfFailedTestCases += TestNaN<40, 8>();

	// six block configurations
	nrOfFailedTestCases += TestNaN<47, 9>();
	nrOfFailedTestCases += TestNaN<48, 9>();

	// seven block configurations
	nrOfFailedTestCases += TestNaN<55, 10>();
	nrOfFailedTestCases += TestNaN<56, 10>();

	// eight block configurations
	nrOfFailedTestCases += TestNaN<63, 11>();
	nrOfFailedTestCases += TestNaN<64, 11>();

	std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
}

void TestSizeof(int& nrOfFailedTestCases) {
	using namespace sw::universal;
	int currentFails = nrOfFailedTestCases;
	std::cout << "sizeof with blocktype uint8_t  : ";
	if constexpr (sizeof(areal<4, 1>(0)) != 1) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<5, 1>(0)) != 1) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<8, 2>(0)) != 1) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<9, 2>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<12, 3>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<16, 5>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<17, 5>(0)) != 3) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<20, 5>(0)) != 3) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<24, 5>(0)) != 3) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<25, 6>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<28, 6>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<32, 8>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<33, 8>(0)) != 5) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<36, 8>(0)) != 5) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<40, 9>(0)) != 5) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<44, 9>(0)) != 6) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<48, 9>(0)) != 6) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<52, 10>(0)) != 7) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<56, 10>(0)) != 7) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<60, 10>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<64, 11>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<72, 11>(0)) != 9) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<80, 11>(0)) != 10) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<88, 11>(0)) != 11) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<96, 11>(0)) != 12) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<104, 11>(0)) != 13) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<112, 11>(0)) != 14) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<120, 11>(0)) != 15) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<128, 11>(0)) != 16) ++nrOfFailedTestCases;
	std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	std::cout << "sizeof with blocktype uint16_t : ";
	if constexpr (sizeof(areal<4, 1, uint16_t>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<5, 1, uint16_t>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<8, 2, uint16_t>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<9, 2, uint16_t>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<12, 3, uint16_t>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<16, 5, uint16_t>(0)) != 2) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<17, 5, uint16_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<20, 5, uint16_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<24, 5, uint16_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<25, 6, uint16_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<28, 6, uint16_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<32, 8, uint16_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<33, 8, uint16_t>(0)) != 6) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<36, 8, uint16_t>(0)) != 6) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<40, 9, uint16_t>(0)) != 6) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<44, 9, uint16_t>(0)) != 6) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<48, 9, uint16_t>(0)) != 6) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<52, 10, uint16_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<56, 10, uint16_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<60, 10, uint16_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<64, 11, uint16_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<72, 11, uint16_t>(0)) != 10) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<80, 11, uint16_t>(0)) != 10) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<88, 11, uint16_t>(0)) != 12) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<96, 11, uint16_t>(0)) != 12) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<104, 11, uint16_t>(0)) != 14) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<112, 11, uint16_t>(0)) != 14) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<120, 11, uint16_t>(0)) != 16) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<128, 11, uint16_t>(0)) != 16) ++nrOfFailedTestCases;
	std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	std::cout << "sizeof with blocktype uint32_t : ";
	if constexpr (sizeof(areal<4, 1, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<5, 1, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<8, 2, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<9, 2, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<12, 3, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<16, 5, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<17, 5, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<20, 5, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<24, 5, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<25, 6, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<28, 6, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<32, 8, uint32_t>(0)) != 4) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<33, 8, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<36, 8, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<40, 9, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<44, 9, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<48, 9, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<52, 10, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<56, 10, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<60, 10, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<64, 11, uint32_t>(0)) != 8) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<72, 11, uint32_t>(0)) != 12) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<80, 11, uint32_t>(0)) != 12) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<88, 11, uint32_t>(0)) != 12) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<96, 11, uint32_t>(0)) != 12) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<104, 11, uint32_t>(0)) != 16) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<112, 11, uint32_t>(0)) != 16) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<120, 11, uint32_t>(0)) != 16) ++nrOfFailedTestCases;
	if constexpr (sizeof(areal<128, 11, uint32_t>(0)) != 16) ++nrOfFailedTestCases;
	std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
}

// TODO: this does not properly test for gradual underflow and gradual overflow
void TestScale(int& nrOfFailedTestCases) {
	using namespace sw::universal;
	int currentFails = nrOfFailedTestCases;

	/*
	an areal is encoded as 1 sign bit, es exponent bits, f fraction bits, and 1 uncertainty bit
	we are specifying just the size, nbits, and the number of exponent bits, es,
	from which we deduce the number of fraction bits, fbits.
	fbits = nbits - 1 sign bit - 1 uncertainty bit - es exponent bits
	fbits > 0 if nbits > es + 2
	thus an areal<3,1> fails that test
	{
		std::cout << "scale areal<3,1>               : ";
		areal<3, 1> a; 
		a.setbits(2); if (a.scale() != 1) ++nrOfFailedTestCases;
		a.setbits(3); if (a.scale() != 1) ++nrOfFailedTestCases;
		a.setbits(5); if (a.scale() != 0) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	*/

	{
		std::cout << "scale areal<4,1>               : ";
		areal<4, 1> a; 
		a.setbits(5); if (a.scale() != 1) ++nrOfFailedTestCases;
		a.setbits(11); if (a.scale() != 0) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");

	}

	{
		std::cout << "scale areal<5,1>               : ";
		areal<5, 1> a;
		a.setbits(12); if (a.scale() != 1) ++nrOfFailedTestCases;
		a.setbits(20); if (a.scale() != 0) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<5,2>               : ";
		areal<5, 2> a;
		// [1-11-11]
		a.setbits(0x1F); if (a.scale() != 2) ++nrOfFailedTestCases;
		// [1-10-11]
		a.setbits(0x1B); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-01-11]
		a.setbits(0x17); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-00-11]
		a.setbits(0x13); if (a.scale() != -1) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<6,1>               : ";
		areal<6, 1> a;
		// [1-1-1111]
		a.setbits(0x3F); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-0-1111]
		a.setbits(0x2F); if (a.scale() != 0) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<7,1>               : ";
		areal<7, 1> a;
		// [1-1-1'1111]
		a.setbits(0x7F); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-0-1'1111]
		a.setbits(0x5F); if (a.scale() != 0) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<8,1>               : ";
		areal<8, 1> a;
		// [1-1-11'1111]
		a.setbits(0xFF); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-0-11'1111]
		a.setbits(0xBF); if (a.scale() != 0) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<8,2>               : ";
		areal<8, 2> a;
		// [1-11-1'1111]
		a.setbits(0xFF); if (a.scale() != 2) ++nrOfFailedTestCases;
		// [1-10-1'1111]
		a.setbits(0xDF); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-01-1'1111]
		a.setbits(0xBF); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-00-1'1111]
		a.setbits(0x9F); if (a.scale() != -1) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<8,3>               : ";
		areal<8, 3> a;
		// [1-111-'1111]
		a.setbits(0xFF); if (a.scale() != 4) ++nrOfFailedTestCases;
		// [1-110-'1111]
		a.setbits(0xEF); if (a.scale() != 3) ++nrOfFailedTestCases;
		// [1-101-'1111]
		a.setbits(0xDF); if (a.scale() != 2) ++nrOfFailedTestCases;
		// [1-100-'1111]
		a.setbits(0xCF); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-011-'1111]
		a.setbits(0xBF); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-010-'1111]
		a.setbits(0xAF); if (a.scale() != -1) ++nrOfFailedTestCases;
		// [1-001-'1111]
		a.setbits(0x9F); if (a.scale() != -2) ++nrOfFailedTestCases;
		// [1-000-'1111]
		a.setbits(0x8F); if (a.scale() != -3) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<8,4>               : ";
		areal<8, 4> a;
		// [1-111'1-111]
		a.setbits(0xFF); if (a.scale() != 8) ++nrOfFailedTestCases;
		// [1-111'0-111]
		a.setbits(0xF7); if (a.scale() != 7) ++nrOfFailedTestCases;
		// [1-110'1-111]
		a.setbits(0xEF); if (a.scale() != 6) ++nrOfFailedTestCases;
		// [1-110'0-111]
		a.setbits(0xE7); if (a.scale() != 5) ++nrOfFailedTestCases;
		// [1-101'1-111]
		a.setbits(0xDF); if (a.scale() != 4) ++nrOfFailedTestCases;
		// [1-101'0-111]
		a.setbits(0xD7); if (a.scale() != 3) ++nrOfFailedTestCases;
		// [1-100'1-111]
		a.setbits(0xCF); if (a.scale() != 2) ++nrOfFailedTestCases;
		// [1-100'0-111]
		a.setbits(0xC7); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-011'1-111]
		a.setbits(0xBF); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-011'0-111]
		a.setbits(0xB7); if (a.scale() != -1) ++nrOfFailedTestCases;
		// [1-010'1-111]
		a.setbits(0xAF); if (a.scale() != -2) ++nrOfFailedTestCases;
		// [1-010'0-111]
		a.setbits(0xA7); if (a.scale() != -3) ++nrOfFailedTestCases;
		// [1-001'1-111]
		a.setbits(0x9F); if (a.scale() != -4) ++nrOfFailedTestCases;
		// [1-001'0-111]
		a.setbits(0x97); if (a.scale() != -5) ++nrOfFailedTestCases;
		// [1-000'1-111]
		a.setbits(0x8F); if (a.scale() != -6) ++nrOfFailedTestCases;
		// [1-000'0-111]
		a.setbits(0x87); if (a.scale() != -7) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}

	{
		std::cout << "scale areal<8,5>               : ";
		areal<8, 5> a;
		// [1-111'11-11]
		a.setbits(0xFF); if (a.scale() != 16) ++nrOfFailedTestCases;
		// [1-111'10-11]
		a.setbits(0xFB); if (a.scale() != 15) ++nrOfFailedTestCases;
		// [1-111'01-11]
		a.setbits(0xF7); if (a.scale() != 14) ++nrOfFailedTestCases;
		// [1-111'00-11]
		a.setbits(0xF3); if (a.scale() != 13) ++nrOfFailedTestCases;
		// [1-110'11-11]
		a.setbits(0xEF); if (a.scale() != 12) ++nrOfFailedTestCases;
		// [1-110'10-11]
		a.setbits(0xEB); if (a.scale() != 11) ++nrOfFailedTestCases;
		// [1-110'01-11]
		a.setbits(0xE7); if (a.scale() != 10) ++nrOfFailedTestCases;
		// [1-110'00-11]
		a.setbits(0xE3); if (a.scale() != 9) ++nrOfFailedTestCases;
		// [1-101'11-11]
		a.setbits(0xDF); if (a.scale() != 8) ++nrOfFailedTestCases;
		// [1-101'10-11]
		a.setbits(0xDB); if (a.scale() != 7) ++nrOfFailedTestCases;
		// [1-101'01-11]
		a.setbits(0xD7); if (a.scale() != 6) ++nrOfFailedTestCases;
		// [1-101'00-11]
		a.setbits(0xD3); if (a.scale() != 5) ++nrOfFailedTestCases;
		// [1-100'11-11]
		a.setbits(0xCF); if (a.scale() != 4) ++nrOfFailedTestCases;
		// [1-100'10-11]
		a.setbits(0xCB); if (a.scale() != 3) ++nrOfFailedTestCases;
		// [1-100'01-11]
		a.setbits(0xC7); if (a.scale() != 2) ++nrOfFailedTestCases;
		// [1-100'00-11]
		a.setbits(0xC3); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-011'11-11]
		a.setbits(0xBF); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-011'10-11]
		a.setbits(0xBB); if (a.scale() != -1) ++nrOfFailedTestCases;
		// [1-011'01-11]
		a.setbits(0xB7); if (a.scale() != -2) ++nrOfFailedTestCases;
		// [1-011'00-11]
		a.setbits(0xB3); if (a.scale() != -3) ++nrOfFailedTestCases;
		// [1-010'11-11]
		a.setbits(0xAF); if (a.scale() != -4) ++nrOfFailedTestCases;
		// [1-010'10-11]
		a.setbits(0xAB); if (a.scale() != -5) ++nrOfFailedTestCases;
		// [1-010'01-11]
		a.setbits(0xA7); if (a.scale() != -6) ++nrOfFailedTestCases;
		// [1-010'00-11]
		a.setbits(0xA3); if (a.scale() != -7) ++nrOfFailedTestCases;
		// [1-001'11-11]
		a.setbits(0x9F); if (a.scale() != -8) ++nrOfFailedTestCases;
		// [1-001'10-11]
		a.setbits(0x9B); if (a.scale() != -9) ++nrOfFailedTestCases;
		// [1-001'01-11]
		a.setbits(0x97); if (a.scale() != -10) ++nrOfFailedTestCases;
		// [1-001'00-11]
		a.setbits(0x93); if (a.scale() != -11) ++nrOfFailedTestCases;
		// [1-000'11-11]
		a.setbits(0x8F); if (a.scale() != -12) ++nrOfFailedTestCases;
		// [1-000'10-11]
		a.setbits(0x8B); if (a.scale() != -13) ++nrOfFailedTestCases;
		// [1-000'01-11]
		a.setbits(0x87); if (a.scale() != -14) ++nrOfFailedTestCases;
		// [1-000'00-11]
		a.setbits(0x83); if (a.scale() != -15) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
#if 0
	{
		areal<9, 2> a; a.scale();
	}
	{
		areal<9, 3> a; a.scale();
	}
	{
		areal<10, 2> a; a.scale();
	}
	{
		areal<10, 3> a; a.scale();
	}
#endif
	std::cout << "\n\nStandard floating-point sizes\n";
	{
		std::cout << "scale areal<8,2,uint8_t>       : ";
		areal<8, 2, uint8_t> a;
		// [1-11-1'1111]
		a.setbits(0xFF); if (a.scale() != 2) ++nrOfFailedTestCases;
		// [1-10-1'1111]
		a.setbits(0xDF); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-01-1'1111]
		a.setbits(0xBF); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-00-1'1111]
		a.setbits(0x9F); if (a.scale() != -1) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<16,5,uint16_t>     : ";
		areal<16, 5, uint16_t> a;
		// [1-111'11-11'0000'0000]
		a.setbits(0xFF00); if (a.scale() != 16) ++nrOfFailedTestCases;
		// [1-111'10-11'0000'0000]
		a.setbits(0xFB00); if (a.scale() != 15) ++nrOfFailedTestCases;
		// [1-111'01-11'0000'0000]
		a.setbits(0xF700); if (a.scale() != 14) ++nrOfFailedTestCases;
		// [1-111'00-11'0000'0000]
		a.setbits(0xF300); if (a.scale() != 13) ++nrOfFailedTestCases;
		// [1-110'11-11'0000'0000]
		a.setbits(0xEF00); if (a.scale() != 12) ++nrOfFailedTestCases;
		// [1-110'10-11'0000'0000]
		a.setbits(0xEB00); if (a.scale() != 11) ++nrOfFailedTestCases;
		// [1-110'01-11'0000'0000]
		a.setbits(0xE700); if (a.scale() != 10) ++nrOfFailedTestCases;
		// [1-110'00-11'0000'0000]
		a.setbits(0xE300); if (a.scale() != 9) ++nrOfFailedTestCases;
		// [1-101'11-11'0000'0000]
		a.setbits(0xDF00); if (a.scale() != 8) ++nrOfFailedTestCases;
		// [1-101'10-11'0000'0000]
		a.setbits(0xDB00); if (a.scale() != 7) ++nrOfFailedTestCases;
		// [1-101'01-11'0000'0000]
		a.setbits(0xD700); if (a.scale() != 6) ++nrOfFailedTestCases;
		// [1-101'00-11'0000'0000]
		a.setbits(0xD300); if (a.scale() != 5) ++nrOfFailedTestCases;
		// [1-100'11-11'0000'0000]
		a.setbits(0xCF00); if (a.scale() != 4) ++nrOfFailedTestCases;
		// [1-100'10-11'0000'0000]
		a.setbits(0xCB00); if (a.scale() != 3) ++nrOfFailedTestCases;
		// [1-100'01-11'0000'0000]
		a.setbits(0xC700); if (a.scale() != 2) ++nrOfFailedTestCases;
		// [1-100'00-11'0000'0000]
		a.setbits(0xC300); if (a.scale() != 1) ++nrOfFailedTestCases;
		// [1-011'11-11'0000'0000]
		a.setbits(0xBF00); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-011'10-11'0000'0000]
		a.setbits(0xBB00); if (a.scale() != -1) ++nrOfFailedTestCases;
		// [1-011'01-11'0000'0000]
		a.setbits(0xB700); if (a.scale() != -2) ++nrOfFailedTestCases;
		// [1-011'00-11'0000'0000]
		a.setbits(0xB300); if (a.scale() != -3) ++nrOfFailedTestCases;
		// [1-010'11-11'0000'0000]
		a.setbits(0xAF00); if (a.scale() != -4) ++nrOfFailedTestCases;
		// [1-010'10-11'0000'0000]
		a.setbits(0xAB00); if (a.scale() != -5) ++nrOfFailedTestCases;
		// [1-010'01-11'0000'0000]
		a.setbits(0xA700); if (a.scale() != -6) ++nrOfFailedTestCases;
		// [1-010'00-11'0000'0000]
		a.setbits(0xA300); if (a.scale() != -7) ++nrOfFailedTestCases;
		// [1-001'11-11'0000'0000]
		a.setbits(0x9F00); if (a.scale() != -8) ++nrOfFailedTestCases;
		// [1-001'10-11'0000'0000]
		a.setbits(0x9B00); if (a.scale() != -9) ++nrOfFailedTestCases;
		// [1-001'01-11'0000'0000]
		a.setbits(0x9700); if (a.scale() != -10) ++nrOfFailedTestCases;
		// [1-001'00-11'0000'0000]
		a.setbits(0x9300); if (a.scale() != -11) ++nrOfFailedTestCases;
		// [1-000'11-11'0000'0000]
		a.setbits(0x8F00); if (a.scale() != -12) ++nrOfFailedTestCases;
		// [1-000'10-11'0000'0000]
		a.setbits(0x8B00); if (a.scale() != -13) ++nrOfFailedTestCases;
		// [1-000'01-11'0000'0000]
		a.setbits(0x8700); if (a.scale() != -14) ++nrOfFailedTestCases;
		// [1-000'00-11'0000'0000]
		a.setbits(0x8300); if (a.scale() != -15) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<32,8,uint32_t>     : ";
		areal<32, 8, uint32_t> a;
		// [1-111'1111'1-111'1111'1111'1111'1111'0000]
		a.setbits(0xFFFF'FFF0); if (a.scale() != 128) ++nrOfFailedTestCases;
		// [1-011'1111'1-111'1111'1111'1111'1111'0000]
		a.setbits(0xBFFF'FFF0); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-000'0000'0-111'1111'1111'1111'1111'0000]
		a.setbits(0x807F'FFF0); if (a.scale() != -127) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<64,11,uint64_t>    : ";
		areal<64, 11, uint64_t> a;
		// [1-111'1111'1111-'1111'1111'1111'1111'0000]
		a.setbits(0xFFFF'FFFF'FFFF'FFF0); 
		if (a.scale() != 1024) ++nrOfFailedTestCases;
		// [1-111'1111'1110-'1111'1111'1111'1111'0000]
		a.setbits(0xFFEF'FFFF'FFFF'FFF0); if (a.scale() != 1023) ++nrOfFailedTestCases;
		// [1-011'1111'1111-'1111'1111'1111'1111'0000]
		a.setbits(0xBFFF'FFFF'FFFF'FFF0); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-000'0000'0000-'1111'1111'1111'1111'0000]
		a.setbits(0x800F'FFFF'FFFF'FFF0); if (a.scale() != -1023) ++nrOfFailedTestCases;
		std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
	}
	{
		std::cout << "scale areal<128,15,uint64_t>   : ";
		//areal<128, 15, uint64_t> a;
		// [1-111'1111'1111'1111-'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'0000]
//		a.assign("0xFFFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFF0"); if (a.scale() != 16*1024) ++nrOfFailedTestCases;
		// [1-011'1111'1111'1111-'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'0000]
//		a.assign("0xBFFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFF0"); if (a.scale() != 0) ++nrOfFailedTestCases;
		// [1-000'0000'0000'0000-'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'0000]
//		a.assign("0x800F'FFFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFF0"); if (a.scale() != -(16*1024 - 1)) ++nrOfFailedTestCases;
		//std::cout << ((currentFails == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");
		std::cout << "TBD\n";
	}
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "areal<> special cases" << std::endl;

#if MANUAL_TESTING

	// scales for gradual overflow range are incorrect
	// also scales for es = 1 are just underflow and overflow ranges, and currently incorrect

	/// TODO: subnormal numbers have a scale adjustment as 2^(2-2^(es - 1)).
	/// check if this is correct if es is > 2. In particular, areal<32,8> and areal<64,11> should write test suite for that

	areal<8, 2> a;
	uint32_t pattern = 0x00000001ul;
	for (unsigned i = 0; i < 23; ++i) {
		a.setbits(pattern);
		std::cout << std::setw(10) << pattern << " " << to_binary(a) << " " << a << std::endl;
		pattern <<= 1;
	}
	GenerateArealTable<8, 2>(std::cout, true);

	int exponents[] = {
		0, 1, 0, -2, -6, -14, -30, -62, -126, -254, -510, -1022
	};
	for (int i = 1; i < 12; ++i) {
		double e{ 0.0 };
		std::cout << "es = " << exponents[i] << " " << std::setprecision(17) << subnormal_exponent[i] << std::endl;
	}

#else // !MANUAL_TESTING

	//bool bReportIndividualTestCases = false;
	//
	TestIsZero(nrOfFailedTestCases);
	TestIsInf(nrOfFailedTestCases);
	TestIsNaN(nrOfFailedTestCases);
	TestSizeof(nrOfFailedTestCases);
	TestScale(nrOfFailedTestCases);

#endif // MANUAL_TESTING

	std::cout << "\nAREAL API test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
