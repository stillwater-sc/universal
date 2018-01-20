// increment.cpp: functional tests for increment operator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <vector>
#include <algorithm>

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;
using namespace sw::unum;

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING
	std::vector< posit<3, 0> > set;
	GenerateOrderedPositSet<3, 0>(set);
	for (typename std::vector< posit<3, 0> >::iterator it = set.begin(); it != set.end(); it++) {
		std::cout << it->get() << " " << *it << std::endl;
	}

#else
	// INCREMENT tests
	cout << endl << "INCREMENT tests" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<3, 0>("Increment failed", bReportIndividualTestCases), "posit<3,0>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<4, 0>("Increment failed", bReportIndividualTestCases), "posit<4,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<4, 1>("Increment failed", bReportIndividualTestCases), "posit<4,1>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<5, 0>("Increment failed", bReportIndividualTestCases), "posit<5,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<5, 1>("Increment failed", bReportIndividualTestCases), "posit<5,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<5, 2>("Increment failed", bReportIndividualTestCases), "posit<5,2>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 0>("Increment failed", bReportIndividualTestCases), "posit<6,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 1>("Increment failed", bReportIndividualTestCases), "posit<6,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 2>("Increment failed", bReportIndividualTestCases), "posit<6,2>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 3>("Increment failed", bReportIndividualTestCases), "posit<6,3>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 0>("Increment failed", bReportIndividualTestCases), "posit<7,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 1>("Increment failed", bReportIndividualTestCases), "posit<7,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 2>("Increment failed", bReportIndividualTestCases), "posit<7,2>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 3>("Increment failed", bReportIndividualTestCases), "posit<7,3>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 0>("Increment failed", bReportIndividualTestCases), "posit<8,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 1>("Increment failed", bReportIndividualTestCases), "posit<8,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 2>("Increment failed", bReportIndividualTestCases), "posit<8,2>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 3>("Increment failed", bReportIndividualTestCases), "posit<8,3>", "operator++");

#endif // MANUAL_TESTING

	if (argc == 2 && std::string(argv[1]) == std::string("-l")) {
		// AD/DA adapted data path configurations
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<10, 0>("Increment failed", bReportIndividualTestCases), "posit<10,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<10, 1>("Increment failed", bReportIndividualTestCases), "posit<10,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<12, 0>("Increment failed", bReportIndividualTestCases), "posit<12,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<12, 1>("Increment failed", bReportIndividualTestCases), "posit<12,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<14, 0>("Increment failed", bReportIndividualTestCases), "posit<14,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<14, 1>("Increment failed", bReportIndividualTestCases), "posit<14,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<15, 0>("Increment failed", bReportIndividualTestCases), "posit<15,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<15, 1>("Increment failed", bReportIndividualTestCases), "posit<15,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<16, 0>("Increment failed", bReportIndividualTestCases), "posit<16,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<16, 1>("Increment failed", bReportIndividualTestCases), "posit<16,1>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<16, 2>("Increment failed", bReportIndividualTestCases), "posit<16,2>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<18, 0>("Increment failed", bReportIndividualTestCases), "posit<18,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<18, 1>("Increment failed", bReportIndividualTestCases), "posit<18,1>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<18, 2>("Increment failed", bReportIndividualTestCases), "posit<18,2>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<20, 1>("Increment failed", bReportIndividualTestCases), "posit<20,1>", "operator++");
		
		// legit float replacement
		//nrOfFailedTestCases += ReportTestResult(ValidateIncrement<24, 1>("Increment failed", bReportIndividualTestCases), "posit<24,1>", "operator++");
		//nrOfFailedTestCases += ReportTestResult(ValidateIncrement<28, 2>("Increment failed", bReportIndividualTestCases), "posit<28,2>", "operator++");

		// legit double replacement
		//nrOfFailedTestCases += ReportTestResult(ValidateIncrement<32, 2>("Increment failed", bReportIndividualTestCases), "posit<32,2>", "operator++");

	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
