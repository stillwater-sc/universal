// multiplication.cpp: test runner for multiplication on adaptive precision binary floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::string test_suite  = "elastic precision floating-point multiplication validation";
	std::string test_tag    = "efloat multiplication";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// Test cases for limb expansion would go here

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
