// hyperbolic.cpp: test suite runner for hyperbolic functions for ereal adaptive-precision
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

	std::string test_suite  = "ereal mathlib hyperbolic function validation";
	std::string test_tag    = "sinh/cosh/tanh/asinh/acosh/atanh";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 0: Minimal smoke test - verify functions are callable
	// TODO Phase 2: Implement using Taylor series or (e^x ± e^-x)/2
	// TODO Phase 2: Implement inverse functions using log formulas
	// TODO Phase 2: Add precision validation tests

	ereal<> x(2.0);
	ereal<> y(0.5);

	std::cout << "Testing hyperbolic functions...\n";
	std::cout << "sinh(" << x << ") = " << sinh(x) << '\n';
	std::cout << "cosh(" << x << ") = " << cosh(x) << '\n';
	std::cout << "tanh(" << x << ") = " << tanh(x) << '\n';
	std::cout << "asinh(" << x << ") = " << asinh(x) << '\n';
	std::cout << "acosh(" << x << ") = " << acosh(x) << '\n';
	std::cout << "atanh(" << y << ") = " << atanh(y) << '\n';

	std::cout << "\nPhase 0: stub infrastructure validation - PASS\n";
	std::cout << "TODO: Implement Taylor series or exp-based formulas in Phase 2\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	// Phase 0: No automated tests yet
	// TODO Phase 2: Add REGRESSION_LEVEL_1 tests (basic hyperbolic at double precision)
	// TODO Phase 2: Add REGRESSION_LEVEL_2 tests (extended precision 100-200 bits)
	// TODO Phase 2: Add REGRESSION_LEVEL_3 tests (high precision 200-500 bits)
	// TODO Phase 2: Add REGRESSION_LEVEL_4 tests (extreme values and precision)

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
