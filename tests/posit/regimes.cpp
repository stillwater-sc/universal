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
	for (int i = -NR_TEST_CASES; i < NR_TEST_CASES+1; i++) {
		int reference = r.regime_size(i);
		int nrRegimeBits = r.assign_regime_pattern(i);	
		if (nrRegimeBits != reference) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) cout << "FAIL: k = " << setw(3) << i << " regime is " << r << " bits " << nrRegimeBits << " reference " << reference << endl;
		}	
		else {
			//if (bReportIndividualTestCases) cout << "PASS: k = " << setw(3) << i << " regime is " << r << " bits " << nrRegimeBits << " reference " << reference << endl;
		}
	}

	return nrOfFailedTestCases;
}

int main()
try {
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

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

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}