// block_div.cpp: functional tests for block binary number division
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include "universal/native/byteArray.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

#include <bitset>

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

//	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "block multiplication: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	//GenerateTestCase<8>(12345, 54321);

	//nrOfFailedTestCases += ReportTestResult(VerifyModularAddition<4>("Manual Testing", true), "array<4,1>", "addition");


#if STRESS_TESTING

#endif

#else

	cout << "block multiplication validation" << endl;



#if STRESS_TESTING



#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
