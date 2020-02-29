// rounding.cpp: functional tests for byte array rounding 
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>

// Configure the fixpnt template environment
// first: enable general or specialized array configurations
#define NATIVE_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define NATIVE_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include "universal/blockbin/blockbinary.hpp"
#include "../utils/blockbinary_helpers.hpp"

std::string roundingDecision(int roundingDirection) {
	std::stringstream ss;
	ss << (roundingDirection == 0 ? "tie" : (roundingDirection > 0 ? "up" : "down"));
	return ss.str();
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular assignment failed: ";

#if MANUAL_TESTING

	blockbinary<8> a, b;
	blockbinary<16> c;
	a = 1;
	b = -1;
	c = urmul(a, b);
	cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(c, true) << endl;



	nrOfFailedTestCases = ReportTestResult(ValidateModularAssignment<4, 3, float>(bReportIndividualTestCases), tag, "fixpnt<4,3>");
	
	// TODO: fixed-point is failing on pure fractional configurations
	//nrOfFailedTestCases = ReportTestResult(ValidateAssignment<4, 4, float>(bReportIndividualTestCases), tag, "fixpnt<4,4>");

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else
	cout << "Fixed-point modular assignment validation" << endl;


//	nrOfFailedTestCases += ReportTestResult(VerifyModularAddition<8, 0>(tag, bReportIndividualTestCases), "fixpnt<8,0>", "addition");

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
