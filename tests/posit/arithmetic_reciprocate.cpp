// arithmetic_reciprocate.cpp: functional tests for arithmetic reciprocation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>

#define POSIT_VERBOSE_OUTPUT

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
void GenerateTestCase(float fa) {
	posit<nbits, es> pa, pref, preciprocal;
	pa = fa;
	pref = -fa;
	preciprocal = -pa;
	cout << "reference " << pref << " result " << preciprocal << endl << endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double da) {
	posit<nbits, es> pa, pref, preciprocal;
	pa = da;
	pref = -da;
	preciprocal = -pa;
	cout << "reference " << pref << " result " << preciprocal << endl << endl;
}


#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;


#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	float fa, fb;
	fa = 0.725f; fb = 0.75f;
	cout << fa << " * " << fb << " = " << fa*fb << endl;
	GenerateTestCase<4, 0>(fa);
	//GenerateTestCase<5, 0>(-0.625f);
	//GenerateTestCase<5, 0>(-0.500f);

#else

	std::string tag = "Reciprocation failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "reciprocation");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "reciprocation");
#endif

#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}

