// 128bit_posit.cpp: Functionality tests for standard 128-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

/*
Standard posits with nbits = 128 have 4 exponent bits.
*/

int main(int argc, char** argv)
try {
	const size_t RND_TEST_CASES = 100000;

	const size_t nbits = 128;
	const size_t es = 4;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<128,4>";

	cout << "Standard posit<128,4> configuration tests" << endl;
	posit<nbits, es> p;
	cout << spec_to_string(p) << endl << endl;

	throw("128bit posits are not supported yet");

	cout << "Arithmetic test randoms" << endl;
	cout << "Addition      :                 " << RND_TEST_CASES << " randoms" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	cout << "Subtraction   :                 " << RND_TEST_CASES << " randoms" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	cout << "Multiplication:                 " << RND_TEST_CASES << " randoms" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	//cout << "Division      :                 " << RND_TEST_CASES << " randoms" << endl;
	//nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<128, 4>(tag, bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");

	return nrOfFailedTestCases;
}
catch (char* e) {
	cerr << e << endl;
	return -1;
}
