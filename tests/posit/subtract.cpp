// subtract.cpp: functional tests for subtraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>

#include "stdafx.h"

#include <vector>

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es>
void GenerateTestCase(float fa, float fb) {
	posit<nbits, es> pa, pb, pref, pdif;
	pa = fa;
	pb = fb;
	pref = fa - fb;
	pdif = pa - pb;
	cout << "reference " << pref << " result " << pdif << endl << endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double da, double db) {
	posit<nbits, es> pa, pb, pref, pdif;
	pa = da;
	pb = db;
	pref = da - db;
	pdif = pa - pb;
	cout << "reference " << pref << " result " << pdif << endl << endl;
}

int main(int argc, char** argv)
try
{
	int nrOfFailedTests = 0;
	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	GenerateTestCase<5, 0>(-0.625f,  4.000f);
	GenerateTestCase<5, 0>(-0.500f,  4.000f);


	nrOfFailedTests += ReportTestResult(ValidateSubtraction<3, 0>("Posit<3,0> subtraction failed: ", bReportIndividualTestCases), "posit<3,0>", "subtraction");

#if 0
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<4, 0>("Posit<4,0> subtraction failed: ", bReportIndividualTestCases), "posit<4,0>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<4, 1>("Posit<4,1> subtraction failed: ", bReportIndividualTestCases), "posit<4,1>", "subtraction");

	nrOfFailedTests += ReportTestResult(ValidateSubtraction<5, 0>("Posit<5,0> subtraction failed: ", bReportIndividualTestCases), "posit<5,0>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<5, 1>("Posit<5,1> subtraction failed: ", bReportIndividualTestCases), "posit<5,1>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<5, 2>("Posit<5,2> subtraction failed: ", bReportIndividualTestCases), "posit<5,2>", "subtraction");

	nrOfFailedTests += ReportTestResult(ValidateSubtraction<6, 0>("Posit<6,0> subtraction failed: ", bReportIndividualTestCases), "posit<6,0>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<6, 1>("Posit<6,1> subtraction failed: ", bReportIndividualTestCases), "posit<6,1>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<6, 2>("Posit<6,2> subtraction failed: ", bReportIndividualTestCases), "posit<6,2>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<6, 3>("Posit<6,3> subtraction failed: ", bReportIndividualTestCases), "posit<6,3>", "subtraction");

	nrOfFailedTests += ReportTestResult(ValidateSubtraction<7, 0>("Posit<7,0> subtraction failed: ", bReportIndividualTestCases), "posit<7,0>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<7, 1>("Posit<7,1> subtraction failed: ", bReportIndividualTestCases), "posit<7,1>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<7, 2>("Posit<7,2> subtraction failed: ", bReportIndividualTestCases), "posit<7,2>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<7, 3>("Posit<7,3> subtraction failed: ", bReportIndividualTestCases), "posit<7,3>", "subtraction");

	nrOfFailedTests += ReportTestResult(ValidateSubtraction<8, 0>("Posit<8,0> subtraction failed: ", bReportIndividualTestCases), "posit<8,0>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<8, 1>("Posit<8,1> subtraction failed: ", bReportIndividualTestCases), "posit<8,1>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<8, 2>("Posit<8,2> subtraction failed: ", bReportIndividualTestCases), "posit<8,2>", "subtraction");
	nrOfFailedTests += ReportTestResult(ValidateSubtraction<8, 3>("Posit<8,3> subtraction failed: ", bReportIndividualTestCases), "posit<8,3>", "subtraction");
#endif

	return nrOfFailedTests;
}
catch (char* e) {
	cerr << e << endl;
	return -1;
}
