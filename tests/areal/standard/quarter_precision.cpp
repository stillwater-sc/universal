// quarter_precision.cpp: test suite runner for quarter-precision floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)
#pragma warning(disable : 4710)
#pragma warning(disable : 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
#include <iostream>
#include <iomanip>
// minimum set of include files to reflect source code dependencies
#include <universal/number/areal/areal_impl.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/areal_test_suite.hpp>
#include <universal/number/areal/table.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) {
		cout << argv[0] << endl;
	}
	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	constexpr size_t nbits = 8;
	constexpr size_t es = 2;

	int nrOfFailedTestCases = 0;
	std::string tag = " areal<8,2>";

	cout << "Standard quarter precision areal<8,2> configuration tests" << endl;

#if MANUAL_TESTING

	bool bReportIndividualTestCases = true;
	{
		using TestType = areal<nbits, es, uint8_t>;
		nrOfFailedTestCases += ExhaustiveNumberSystemTest<TestType>(tag, bReportIndividualTestCases);
	}


#else // !MANUAL_TESTING

	bool bReportIndividualTestCases = false;
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<TestType>(bReportIndividualTestCases);

#endif // MANUAL_TESTING

	if (nrOfFailedTestCases) {
		cout << tag << " tests FAIL: " << nrOfFailedTestCases << " failures\n";
	}
	else {
		cout << tag << " tests PASS\n";
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
