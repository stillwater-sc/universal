// api.cpp: application programming interface tests for areal number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
#include <iostream>
#include <iomanip>
#include <fstream>
#include <typeinfo>
// minimum set of include files to reflect source code dependencies
#include <universal/number/areal/areal.hpp>
#include <universal/number/areal/manipulators.hpp>  // hex_print and the like
#include <universal/verification/test_suite_arithmetic.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 0) {
		std::cout << argv[0] << std::endl;
	}
	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	int nrOfFailedTestCases = 0;

	std::cout << "areal<> Application Programming Interface tests" << std::endl;

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

#else // !MANUAL_TESTING

	//bool bReportIndividualTestCases = false;

#endif // MANUAL_TESTING

	std::cout << "\nAREAL API test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

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
