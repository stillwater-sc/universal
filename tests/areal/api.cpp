// api.cpp: application programming interface tests for areal number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma warning(disable : 4514)
#pragma warning(disable : 4710)
// minimum set of include files to reflect source code dependencies
#include <universal/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

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
	std::cout << "iszero(): ";
	// one block configurations
	nrOfFailedTestCases += TestZero<3, 1>();
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
	r = -r;
	//std::cout << to_binary(r) << std::endl;
	if (!r.isinf()) ++fails;
	return fails;
}

void TestIsInf(int& nrOfFailedTestCases) {
	int currentFails = nrOfFailedTestCases;
	std::cout << "isinf(): ";
	// one block configurations
	nrOfFailedTestCases += TestInf<3, 1>();
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
	//std::cout << to_binary(r) << std::endl;
	if (!r.isnan()) ++fails;
	r = -r;
	//std::cout << to_binary(r) << std::endl;
	if (!r.isnan()) ++fails;
	return fails;
}

void TestIsNaN(int& nrOfFailedTestCases) {
	int currentFails = nrOfFailedTestCases;
	std::cout << "iszero(): ";
	// one block configurations
	nrOfFailedTestCases += TestNaN<3, 1>();
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

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 0) {
		std::cout << argv[0] << std::endl;
	}
	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	constexpr size_t nbits = 8;
	constexpr size_t es = 2;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;
	std::string tag = " areal<8,2>";

	std::cout << "areal<> Application Programming Interface tests" << std::endl;

#if MANUAL_TESTING

	TestIsZero(nrOfFailedTestCases);
	TestIsInf(nrOfFailedTestCases);

#else // !MANUAL_TESTING


#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
