// decrement.cpp: functional tests for decrement operator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <vector>
#include <algorithm>

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	// DECREMENT tests
	cout << endl << "DECREMENT tests" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<3, 0>("Decrement failed", bReportIndividualTestCases), "posit<3,0>", "operator--");

	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<4, 0>("Decrement failed", bReportIndividualTestCases), "posit<4,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<4, 1>("Decrement failed", bReportIndividualTestCases), "posit<4,1>", "operator--");

	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<5, 0>("Decrement failed", bReportIndividualTestCases), "posit<5,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<5, 1>("Decrement failed", bReportIndividualTestCases), "posit<5,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<5, 2>("Decrement failed", bReportIndividualTestCases), "posit<5,2>", "operator--");

	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 0>("Decrement failed", bReportIndividualTestCases), "posit<6,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 1>("Decrement failed", bReportIndividualTestCases), "posit<6,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 2>("Decrement failed", bReportIndividualTestCases), "posit<6,2>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 3>("Decrement failed", bReportIndividualTestCases), "posit<6,3>", "operator--");

	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 0>("Decrement failed", bReportIndividualTestCases), "posit<7,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 1>("Decrement failed", bReportIndividualTestCases), "posit<7,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 2>("Decrement failed", bReportIndividualTestCases), "posit<7,2>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 3>("Decrement failed", bReportIndividualTestCases), "posit<7,3>", "operator--");

	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 0>("Decrement failed", bReportIndividualTestCases), "posit<8,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 1>("Decrement failed", bReportIndividualTestCases), "posit<8,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 2>("Decrement failed", bReportIndividualTestCases), "posit<8,2>", "operator--");
	nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 3>("Decrement failed", bReportIndividualTestCases), "posit<8,3>", "operator--");

	// long running
	if (argc == 2 && std::string(argv[1]) == std::string("-l")) {
		// AD/DA adapted data path configurations
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<10, 0>("Decrement failed", bReportIndividualTestCases), "posit<10,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<10, 1>("Decrement failed", bReportIndividualTestCases), "posit<10,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<12, 0>("Decrement failed", bReportIndividualTestCases), "posit<12,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<12, 1>("Decrement failed", bReportIndividualTestCases), "posit<12,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<14, 0>("Decrement failed", bReportIndividualTestCases), "posit<14,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<14, 1>("Decrement failed", bReportIndividualTestCases), "posit<14,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<15, 0>("Decrement failed", bReportIndividualTestCases), "posit<15,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<15, 1>("Decrement failed", bReportIndividualTestCases), "posit<15,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<16, 0>("Decrement failed", bReportIndividualTestCases), "posit<16,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<16, 1>("Decrement failed", bReportIndividualTestCases), "posit<16,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<16, 2>("Decrement failed", bReportIndividualTestCases), "posit<16,2>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<18, 0>("Decrement failed", bReportIndividualTestCases), "posit<18,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<18, 1>("Decrement failed", bReportIndividualTestCases), "posit<18,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<18, 2>("Decrement failed", bReportIndividualTestCases), "posit<18,2>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<20, 1>("Decrement failed", bReportIndividualTestCases), "posit<20,1>", "operator--");

		// legit float replacement
		//nrOfFailedTestCases += ReportTestResult(ValidateDecrement<24, 1>("Decrement failed", bReportIndividualTestCases), "posit<24,1>", "operator--");
		//nrOfFailedTestCases += ReportTestResult(ValidateDecrement<28, 1>("Decrement failed", bReportIndividualTestCases), "posit<28,2>", "operator--");

		// legit double replacement
		//nrOfFailedTestCases += ReportTestResult(ValidateDecrement<32, 2>("Decrement failed", bReportIndividualTestCases), "posit<32,2>", "operator--");
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
