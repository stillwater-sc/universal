// prefix.cpp functional tests for prefix operators
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
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<3, 0>("Increment failed", bReportIndividualTestCases), "posit<3,0>", "prefix++(posit)");

	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<4, 0>("Increment failed", bReportIndividualTestCases), "posit<4,0>", "prefix++(posit)");
	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<4, 1>("Increment failed", bReportIndividualTestCases), "posit<4,1>", "prefix++(posit)");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
