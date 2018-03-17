// 128bit_posit.cpp: Functionality tests for standard 128-bit posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

#include <vector>
#include <posit>

#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

/*
Standard posits with nbits = 128 have 4 exponent bits.
*/

int main(int argc, char** argv)
#if 0                                                       // Deal with this later
try {
	using namespace std;
	using namespace sw::unum;

	const size_t RND_TEST_CASES = 10000;

	const size_t nbits = 128;
	const size_t es = 4;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<128,4>";

	cout << "Standard posit<128,4> configuration tests" << endl;
	posit<nbits, es> p;
	cout << spec_to_string(p) << endl << endl;

	throw("128bit posits are not yet supported");

	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
#else
{
    return 0;
}
#endif