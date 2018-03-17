// 8bit_posit.cpp: Functionality tests for standard 8-bit posits
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
Standard posits with nbits = 8 have no exponent bits, i.e. es = 0.
*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	const size_t nbits = 8;
	const size_t es = 0;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<8,0>";

	cout << "Standard posit<8,0> configuration tests" << endl;

	posit<nbits,es> p;
	cout << spec_to_string(p) << endl;

	nrOfFailedTestCases = ReportTestResult( ValidateAddition      <nbits, es>(tag, bReportIndividualTestCases), tag, "add        ") ;
	nrOfFailedTestCases = ReportTestResult( ValidateSubtraction   <nbits, es>(tag, bReportIndividualTestCases), tag, "subtract   ");
	nrOfFailedTestCases = ReportTestResult( ValidateMultiplication<nbits, es>(tag, bReportIndividualTestCases), tag, "multiply   ");
	nrOfFailedTestCases = ReportTestResult( ValidateDivision      <nbits, es>(tag, bReportIndividualTestCases), tag, "divide     ");
	nrOfFailedTestCases = ReportTestResult( ValidateNegation      <nbits, es>(tag, bReportIndividualTestCases), tag, "negate     ");
	nrOfFailedTestCases = ReportTestResult( ValidateReciprocation <nbits, es>(tag, bReportIndividualTestCases), tag, "reciprocate");


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
