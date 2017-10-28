// 8bit_posit.cpp: Functionality tests for standard 8-bit posits
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
Standard posits with nbits = 8 have no exponent bits.
*/

int main(int argc, char** argv)
try
{
	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	cout << "Standard posit<8,0> configuration tests" << endl;

    const size_t nbits = 8;
    const size_t es = 0;
    posit<nbits,es> p;

	cout << spec_to_string(p) << endl;

	nrOfFailedTestCases = ReportTestResult(ValidateAddition<8,0>("Posit<8,0> addition failed: ", bReportIndividualTestCases), "posit<8,0>", "addition") ;
	return nrOfFailedTestCases;
}
catch (char* e) {
	cerr << e << endl;
	return -1;
}
