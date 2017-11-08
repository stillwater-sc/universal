// arithmetic_multiply.cpp: functional tests for multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>

//#define POSIT_VERBOSE_OUTPUT

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_mul
template<size_t nbits, size_t es>
void GenerateTestCase(float fa, float fb) {
	posit<nbits, es> pa, pb, pref, pmul;
	pa = fa;
	pb = fb;
	pref = fa * fb;
	pmul = pa * pb;
	cout << "reference " << pref << " result " << pmul << endl << endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double da, double db) {
	posit<nbits, es> pa, pb, pref, pmul;
	pa = da;
	pb = db;
	pref = da * db;
	pmul = pa * pb;
	cout << "reference " << pref << " result " << pmul << endl << endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {	
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;
	
	std::string tag = "Multiplication failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	float fa, fb;
	fa = 0.75f; fb = 0.75f;
	cout << fa << " * " << fb << " = " << fa*fb << endl;
	GenerateTestCase<4,0>(fa, fb);

	//nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 0>("Manual Testing: ", true), "posit<4,0>", "multiplication");

#else


	cout << "Posit multiplication validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "multiplication");



#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "multiplication");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}

