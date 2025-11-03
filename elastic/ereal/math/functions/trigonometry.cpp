// trigonometry.cpp: test suite runner for trigonometric functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite  = "ereal mathlib trigonometric function validation";
	std::string test_tag    = "sin/cos/tan/asin/acos/atan/atan2";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 0: Minimal smoke test - verify functions are callable
	// TODO Phase 3: Implement using Taylor series with argument reduction
	// TODO Phase 3: Implement inverse functions using Taylor series or Newton iteration
	// TODO Phase 3: Add precision validation tests
	// TODO Phase 3: Add argument reduction tests (large angles)

	ereal<> x(1.0);
	ereal<> y(0.5);

	std::cout << "Testing trigonometric functions...\n";
	std::cout << "sin(" << x << ") = " << sin(x) << '\n';
	std::cout << "cos(" << x << ") = " << cos(x) << '\n';
	std::cout << "tan(" << x << ") = " << tan(x) << '\n';
	std::cout << "asin(" << y << ") = " << asin(y) << '\n';
	std::cout << "acos(" << y << ") = " << acos(y) << '\n';
	std::cout << "atan(" << x << ") = " << atan(x) << '\n';
	std::cout << "atan2(" << x << ", " << y << ") = " << atan2(x, y) << '\n';

	std::cout << "\nPhase 0: stub infrastructure validation - PASS\n";
	std::cout << "TODO: Implement Taylor series with argument reduction in Phase 3\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	// Phase 0: No automated tests yet
	// TODO Phase 3: Add REGRESSION_LEVEL_1 tests (basic trig at double precision)
	// TODO Phase 3: Add REGRESSION_LEVEL_2 tests (extended precision, argument reduction)
	// TODO Phase 3: Add REGRESSION_LEVEL_3 tests (high precision 200-500 bits)
	// TODO Phase 3: Add REGRESSION_LEVEL_4 tests (extreme values and precision)

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
