// arithmetic_negate.cpp: functional tests for arithmetic negation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
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
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Negation failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	//GenerateTestCase<5, 0>(-0.625f);
	//GenerateTestCase<5, 0>(-0.500f);

	//nrOfFailedTestCases += ReportTestResult(ValidateNegation<5, 0>("Manual Testing: ", true), "posit<5,0>", "multiplication");

#else


	cout << "Posit negation validation" << endl;


	nrOfFailedTestCases += ReportTestResult(ValidateNegation<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "negation");

	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "negation");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(ValidateNegation<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "negation");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING
	
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}

