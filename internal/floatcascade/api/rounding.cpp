// rounding.cpp: functional tests for rounding using floatcascade types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <sstream>

#include <universal/internal/floatcascade/floatcascade.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw::universal {
    // report helper to interpret rounding decision encoding
    std::string roundingDecision(int roundingDirection) {
	    std::stringstream ss;
	    ss << (roundingDirection == 0 ? "tie" : (roundingDirection > 0 ? "up" : "down"));
	    return ss.str();
    }

    template<size_t N>
    int ValidateSpecialRoundingCases(bool reportTestCases) {
	    int nrTestFailures = 0;

	    return nrTestFailures;
    }

    template<size_t N>
    int ValidateRounding(bool reportTestCases) {
	    int nrTestFailures = 0;

	    return nrTestFailures;
    }

}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	
	std::string test_suite  = "floatcascade rounding validation";
	std::string test_tag    = "rounding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	//nrOfFailedTestCases += ValidateAny(reportTestCases);

	// basic algorithm
	//     ...


	nrOfFailedTestCases = ReportTestResult(ValidateSpecialRoundingCases<2>(reportTestCases), test_tag, "special rounding cases");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(ValidateSpecialRoundingCases<2>(reportTestCases), test_tag, "special rounding cases");
	nrOfFailedTestCases = ReportTestResult(ValidateRounding<2>(reportTestCases), test_tag, "arithmetic rounding cases");

#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
