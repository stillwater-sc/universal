// next.cpp: test suite runner for nextafter/nexttoward functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

	// Verify nextafter function
    template<typename Real>
    int VerifyNextafter(bool reportTestCases) {
	    int    nrOfFailedTestCases = 0;
	    // double error_mag;

	    Real x, y;
	    Real result, expected;

	    // Test: nextafter(0, 0) = 0
	    x         = 0.0;
	    y         = 0.0;
	    expected  = 0.0;
	    result    = nextafter(x, y);

	    // error_mag = std::abs(double(result - expected));
	    if (!result.iszero()) {
		    if (reportTestCases)
			    std::cerr << "FAIL: nextafter(0, 0) != 0\n";
		    ++nrOfFailedTestCases;
	    }

		// Test: nextafter(1.0, 2.0) = 1.0 + ulp(1.0)
	    x         = 1.0;
	    y         = 2.0;
	    expected  = std::nextafter(1.0, 2.0);
	    result    = nextafter(x, y);
	    if (result != expected) {
		    if (reportTestCases) {
			    std::cerr << "FAIL: nextafter(1.0, 2.0) != 1.0 + ulp(1.0)\n";
			    std::cerr << "  expected: " << to_binary(expected) << " : " << expected << '\n';
			    std::cerr << "    result: " << to_binary(result) << " : " << result << '\n';
		    }
		    ++nrOfFailedTestCases;
	    }

		// Test: nextafter(1.0, 0.5) = 1.0 - ulp(1.0)
	    x        = 1.0;
	    y        = 0.5;
	    expected = std::nextafter(1.0, 0.5);
	    result   = nextafter(x, y);
	    if (result != expected) {
		    if (reportTestCases) {
			    std::cerr << "FAIL: nextafter(1.0, 0.5) != 1.0 - ulp(1.0)\n";
				std::cerr << "  expected: " << to_binary(expected) << " : " << expected << '\n';
				std::cerr << "    result: " << to_binary(result) << " : " << result << '\n';
			}
		    ++nrOfFailedTestCases;
	    }

	    return nrOfFailedTestCases;
    }

	} // namespace universal
 }  // namespace sw

    // Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite  = "ereal mathlib nextafter/nexttoward function validation";
	std::string test_tag    = "nextafter/nexttoward";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 0: Minimal smoke test - verify functions are callable
	// TODO Phase 1: Implement using expansion arithmetic component manipulation
	// TODO Phase 1: Note: For adaptive precision, "next" may involve adding a small limb
	// TODO Phase 2: Add precision validation tests
	// TODO Phase 2: Verify adaptive behavior (precision grows when needed)

	ereal<> x(2.0);
	ereal<> y(3.0);

	std::cout << "Testing next functions...\n";
	double ref = std::nextafter(double(x), double(y));
	double next = double(nextafter(x, y));
	std::cout << "reference: " << to_binary(ref) << " : " << ref << '\n';
	std::cout << "computed : " << to_binary(next) << " : " << next << '\n';

	nrOfFailedTestCases +=
	    ReportTestResult(VerifyNextafter<ereal<>>(reportTestCases), "nextafter(ereal, ereal)", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#	if REGRESSION_LEVEL_1
	// Basic nextafter/nexttoward functionality
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyNextafter<ereal<>>(reportTestCases), "nextafter(ereal, ereal)", test_tag);

#	endif

#	if REGRESSION_LEVEL_2
	// Extended precision nextafter/nexttoward functionality
#	endif

#	if REGRESSION_LEVEL_3
	// Extreme precision nextafter/nexttoward functionality

#	endif

#	if REGRESSION_LEVEL_4
	// Stress nextafter/nexttoward functionality
#	endif


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
