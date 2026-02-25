// api.cpp: application programming interface tests for areal number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::cout << "areal<> Application Programming Interface tests" << std::endl;
	int nrOfFailedTestCases = 0;


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
