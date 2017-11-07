// arithmetic_negate.cpp: functional tests for arithmetic negation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es>
void GenerateTestCase(float fa) {
	posit<nbits, es> pa, pref, pneg;
	pa = fa;
	pref = -fa;
	pneg = -pa;
	cout << "reference " << pref << " result " << pneg << endl << endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double da) {
	posit<nbits, es> pa, pref, pneg;
	pa = da;
	pref = -da;
	pneg = -pa;
	cout << "reference " << pref << " result " << pneg << endl << endl;
}

int main(int argc, char** argv)
try {
	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	//GenerateTestCase<5, 0>(-0.625f);
	//GenerateTestCase<5, 0>(-0.500f);

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<3, 0>("Posit<3,0> negation failed: ", bReportIndividualTestCases), "posit<3,0>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<4, 0>("Posit<4,0> negation failed: ", bReportIndividualTestCases), "posit<4,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<4, 1>("Posit<4,1> negation failed: ", bReportIndividualTestCases), "posit<4,1>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<5, 0>("Posit<5,0> negation failed: ", bReportIndividualTestCases), "posit<5,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<5, 1>("Posit<5,1> negation failed: ", bReportIndividualTestCases), "posit<5,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<5, 2>("Posit<5,2> negation failed: ", bReportIndividualTestCases), "posit<5,2>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 0>("Posit<6,0> negation failed: ", bReportIndividualTestCases), "posit<6,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 1>("Posit<6,1> negation failed: ", bReportIndividualTestCases), "posit<6,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 2>("Posit<6,2> negation failed: ", bReportIndividualTestCases), "posit<6,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 3>("Posit<6,3> negation failed: ", bReportIndividualTestCases), "posit<6,3>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 0>("Posit<7,0> negation failed: ", bReportIndividualTestCases), "posit<7,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 1>("Posit<7,1> negation failed: ", bReportIndividualTestCases), "posit<7,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 2>("Posit<7,2> negation failed: ", bReportIndividualTestCases), "posit<7,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 3>("Posit<7,3> negation failed: ", bReportIndividualTestCases), "posit<7,3>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 0>("Posit<8,0> negation failed: ", bReportIndividualTestCases), "posit<8,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 1>("Posit<8,1> negation failed: ", bReportIndividualTestCases), "posit<8,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 2>("Posit<8,2> negation failed: ", bReportIndividualTestCases), "posit<8,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 3>("Posit<8,3> negation failed: ", bReportIndividualTestCases), "posit<8,3>", "negation");


	nrOfFailedTestCases += ReportTestResult(ValidateNegation<16, 1>("Posit<16,1> negation failed: ", bReportIndividualTestCases), "posit<16,1>", "negation");
	
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}

