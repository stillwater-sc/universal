// 8bit_posit.cpp: Functionality tests for standard 8-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>
#include <posit>

#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;
using namespace sw::unum;

/*
Standard posits with nbits = 8 have no exponent bits.
*/

int main(int argc, char** argv)
try {
	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	cout << "Standard posit<8,0> configuration tests" << endl;

    const size_t nbits = 8;
    const size_t es = 0;
    posit<nbits,es> p;

	cout << spec_to_string(p) << endl;

	nrOfFailedTestCases = ReportTestResult(ValidateAddition<8,0>("Posit<8,0> addition failed: ", bReportIndividualTestCases), "posit<8,0>", "add") ;
	nrOfFailedTestCases = ReportTestResult(ValidateSubtraction<8, 0>("Posit<8,0> subtraction failed: ", bReportIndividualTestCases), "posit<8,0>", "subtract");
	nrOfFailedTestCases = ReportTestResult(ValidateMultiplication<8, 0>("Posit<8,0> multiplication failed: ", bReportIndividualTestCases), "posit<8,0>", "multiply");
	nrOfFailedTestCases = ReportTestResult(ValidateDivision<8, 0>("Posit<8,0> division failed: ", bReportIndividualTestCases), "posit<8,0>", "divide");
	nrOfFailedTestCases = ReportTestResult(ValidateNegation<8, 0>("Posit<8,0> negation failed: ", bReportIndividualTestCases), "posit<8,0>", "negate");
	nrOfFailedTestCases = ReportTestResult(ValidateReciprocation<8, 0>("Posit<8,0> reciprocation failed: ", bReportIndividualTestCases), "posit<8,0>", "reciprocate");


	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
