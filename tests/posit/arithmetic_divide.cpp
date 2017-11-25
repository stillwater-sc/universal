// arithmetic_divide.cpp: functional tests for division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>

//#define POSIT_VERBOSE_OUTPUT

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es>
void GenerateTestCase(float fa, float fb) {
	posit<nbits, es> pa, pb, pref, pdiv;
	pa = fa;
	pb = fb;
	pref = fa / fb;
	pdiv = pa / pb;
	cout << "reference " << pref << " result " << pdiv << endl << endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double da, double db) {
	posit<nbits, es> pa, pb, pref, pdiv;
	pa = da;
	pb = db;
	pref = da / db;
	pdiv = pa / pb;
	cout << "reference " << pref << " result " << pdiv << endl << endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Division failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<5, 0>(4.000f, -2.0f);
	GenerateTestCase<5, 0>(4.000f,  0.5f);

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 0>("Manual Testing", true), "posit<3,0>", "division");

#else

	cout << "Posit division validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "division");


#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "division");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}

