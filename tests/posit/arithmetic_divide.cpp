// arithmetic_divide.cpp: functional tests for division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

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

int main(int argc, char** argv)
try 
{
	int nrOfFailedTests = 0;
	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	//GenerateTestCase<5, 0>(-0.625f, 4.000f);
	//GenerateTestCase<5, 0>(-0.500f, 4.000f);

	nrOfFailedTests += ReportTestResult(ValidateDivision<3, 0>("Posit<3,0> division failed: ", bReportIndividualTestCases), "posit<3,0>", "division");

	nrOfFailedTests += ReportTestResult(ValidateDivision<4, 0>("Posit<4,0> division failed: ", bReportIndividualTestCases), "posit<4,0>", "division");
	nrOfFailedTests += ReportTestResult(ValidateDivision<4, 1>("Posit<4,1> division failed: ", bReportIndividualTestCases), "posit<4,1>", "division");

	return nrOfFailedTests;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
