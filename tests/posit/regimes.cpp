//  regimes.cpp : tests on posit regimes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"

using namespace std;

/*
Regime range example for a posit<6,es>
     regime      scale
     00000          ~   value is either 0 or inf
	 00001         -4
	 0001-         -3
	 001--         -2
	 01---         -1
	 10---          0
	 110--          1
	 1110-          2
	 11110          3
	 11111          4
*/
template<size_t nbits, size_t es>
int ValidateRegimeOperations(std::string tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = nbits;
	int nrOfFailedTestCases = 0;

	regime<nbits, es> r;
	for (int k = -NR_TEST_CASES; k < NR_TEST_CASES+1; k++) {
		int reference = r.regime_size(k);
		int nrRegimeBits = r.assign_regime_pattern(k);	
		if (nrRegimeBits != reference) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) cout << "FAIL: k = " << setw(3) << k << " regime is " << r << " bits " << nrRegimeBits << " reference " << reference << endl;
		}	
		else {
			//if (bReportIndividualTestCases) cout << "PASS: k = " << setw(3) << k << " regime is " << r << " bits " << nrRegimeBits << " reference " << reference << endl;
		}
	}

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;


#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	regime<10, 2> r;
	for (int k = -7; k < 9; k++) {
		int regime_size = r.assign_regime_pattern(k);
		cout << "k = " << setw(3) << k << " regime is " << r << " regime size is " << regime_size << " bits" << endl;
	}

#else
	std::string tag = "Regime conversion failed";

	cout << "Regime tests" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<3, 0>(tag, bReportIndividualTestCases), "regime<3,0>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<4, 0>(tag, bReportIndividualTestCases), "regime<4,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<4, 1>(tag, bReportIndividualTestCases), "regime<4,1>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<5, 0>(tag, bReportIndividualTestCases), "regime<5,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<5, 1>(tag, bReportIndividualTestCases), "regime<5,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<5, 2>(tag, bReportIndividualTestCases), "regime<5,2>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 0>(tag, bReportIndividualTestCases), "regime<6,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 1>(tag, bReportIndividualTestCases), "regime<6,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 2>(tag, bReportIndividualTestCases), "regime<6,2>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 3>(tag, bReportIndividualTestCases), "regime<6,3>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 0>(tag, bReportIndividualTestCases), "regime<7,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 1>(tag, bReportIndividualTestCases), "regime<7,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 2>(tag, bReportIndividualTestCases), "regime<7,2>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 3>(tag, bReportIndividualTestCases), "regime<7,3>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 0>(tag, bReportIndividualTestCases), "regime<8,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 1>(tag, bReportIndividualTestCases), "regime<8,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 2>(tag, bReportIndividualTestCases), "regime<8,2>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 3>(tag, bReportIndividualTestCases), "regime<8,3>", "regime");

#endif


	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}